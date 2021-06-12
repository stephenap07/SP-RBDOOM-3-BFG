/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2021 Stephen Pridham

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#pragma hdrstop

#include "precompiled.h"

#include "D3RmlRender.h"

#include "renderer/GuiModel.h"
#include "renderer/GLState.h"
#include "renderer/Image.h"

#include "rmlui/RmlUserInterfaceLocal.h"

extern RmlUserInterfaceManagerLocal rmlManagerLocal;

extern idDeviceContext* rmlDc;

/*
===============
idRmlRender

Front end rendering for RML
===============
*/

constexpr int kMaxInitialQuads = 1024;
constexpr int kMaxInitialVerts = kMaxInitialQuads * 4;
constexpr int kMaxInitialTris = kMaxInitialQuads * 6;

constexpr int kRmlStencilRef = 128;
constexpr int kRmlStencilMask = 255;

extern idGuiModel* tr_guiModel;

idRmlRender::idRmlRender()
	: _enableScissor( false )
	, _clipRects()
	, _cursorImages()
	, _numMasks( 0 )
	, _verts( nullptr )
	, _tris( nullptr )
	, _texGen( 0 )
	, _numVerts( 0 )
	, _numIndexes( 0 )
{
}

idRmlRender::~idRmlRender()
{
	delete[] _verts;
	delete[] _tris;
}

void idRmlRender::Init()
{
	_verts = new idDrawVert[kMaxInitialVerts];
	_tris = new triIndex_t[kMaxInitialTris];

	origin.Zero();
	mat.Identity();

	_guiSolid = declManager->FindMaterial( "_white", false );
}

void idRmlRender::RenderGeometry( Rml::Vertex* vertices, int numVerts, int* indices, int numIndexes, Rml::TextureHandle texture, const Rml::Vector2f& translation )
{
	triIndex_t* tris = &_tris[_numIndexes];

	for( int i = 0; i < numIndexes; i++ )
	{
		tris[i] = indices[i];
		_numIndexes++;

		if( _numIndexes > kMaxInitialTris )
		{
			// Possibly just make this dynamic
			common->FatalError( "Increase kMaxInitialTris" );
			return;
		}
	}

	const idVec2 scaleToVirtual( ( float )renderSystem->GetVirtualWidth() / renderSystem->GetWidth(),
								 ( float )renderSystem->GetVirtualHeight() / renderSystem->GetHeight() );

	idDrawVert* temp = &_verts[_numVerts];
	for( int i = 0; i < numVerts; i++ )
	{
		idVec3 pos = idVec3( vertices[i].position.x, vertices[i].position.y, 0 );
		pos += idVec3( translation.x, translation.y, 0 );

		// transform
		pos *= mat;
		pos += origin;

		pos.x *= scaleToVirtual.x;
		pos.y *= scaleToVirtual.y;

		temp[i].xyz = pos;
		temp[i].SetTexCoord( vertices[i].tex_coord.x, vertices[i].tex_coord.y );
		temp[i].SetColor( PackColor( idVec4( vertices[i].colour.red, vertices[i].colour.blue, vertices[i].colour.green, vertices[i].colour.alpha ) ) );
		temp[i].SetColor2( PackColor( idVec4( vertices[i].colour.red, vertices[i].colour.blue, vertices[i].colour.green, vertices[i].colour.alpha ) ) );

		_numVerts++;

		if( _numVerts > kMaxInitialVerts )
		{
			common->FatalError( "Increase kMaxInitialVerts" );
			return;
		}
	}

	uint64_t glState = 0;

	if( _enableScissor )
	{
		glState = GLS_DEPTHFUNC_LESS | GLS_DEPTHMASK | GLS_STENCIL_FUNC_GEQUAL | GLS_STENCIL_MAKE_REF( kRmlStencilRef - _numMasks ) | GLS_STENCIL_MAKE_MASK( kRmlStencilMask );
	}

	const idMaterial* material = reinterpret_cast<const idMaterial*>( texture );

	if( material )
	{
		material->SetSort( SS_GUI );
	}

	idDrawVert* verts = tr_guiModel->AllocTris(
							numVerts,
							tris,
							numIndexes,
							material,
							glState,
							STEREO_DEPTH_TYPE_NONE );

	if( verts )
	{
		WriteDrawVerts16( verts, temp, numVerts );
	}

	_numVerts = 0;
	_numIndexes = 0;
}

void idRmlRender::SetTransform( const Rml::Matrix4f* transform )
{
	origin.Zero();
	mat.Identity();

	if( !transform )
	{
		return;
	}

	auto rmlVec1 = transform->GetColumn( 0 );
	auto rmlVec2 = transform->GetColumn( 1 );
	auto rmlVec3 = transform->GetColumn( 2 );
	auto rmlVec4 = transform->GetColumn( 3 );

	origin.Set( rmlVec4.x, rmlVec4.y, rmlVec4.z );

	idVec3 vecX( rmlVec1.x, rmlVec1.y, rmlVec1.z );
	idVec3 vecY( rmlVec2.x, rmlVec2.y, rmlVec2.z );
	idVec3 vecZ( rmlVec3.x, rmlVec3.y, rmlVec3.z );

	mat = idMat3( vecX, vecY, vecZ );
}

static triIndex_t quadPicIndexes[6] = { 3, 0, 2, 2, 0, 1 };

void idRmlRender::RenderClipMask()
{
	// Usually, scissor regions are handled  with actual scissor render commands.
	// We're using stencil masks to do the same thing because it works in worldspace a
	// lot better than screen space scissor rects.
	const idVec2 scaleToVirtual( ( float )renderSystem->GetVirtualWidth() / renderSystem->GetWidth(),
								 ( float )renderSystem->GetVirtualHeight() / renderSystem->GetHeight() );

	ALIGNTYPE16 idDrawVert localVerts[4];

	localVerts[0].Clear();
	localVerts[0].xyz[0] = _clipRects.x;
	localVerts[0].xyz[1] = _clipRects.y;
	localVerts[0].xyz[2] = 0.0f;

	localVerts[0].xyz *= mat;
	localVerts[0].xyz += origin;

	localVerts[0].xyz.x *= scaleToVirtual.x;
	localVerts[0].xyz.y *= scaleToVirtual.y;

	localVerts[0].SetTexCoord( 0.0f, 1.0f );
	localVerts[0].SetColor( PackColor( idVec4() ) );
	localVerts[0].ClearColor2();

	localVerts[1].Clear();
	localVerts[1].xyz[0] = _clipRects.x + _clipRects.w ;
	localVerts[1].xyz[1] = _clipRects.y;
	localVerts[1].xyz[2] = 0.0f;

	localVerts[1].xyz *= mat;
	localVerts[1].xyz += origin;

	localVerts[1].xyz.x *= scaleToVirtual.x;
	localVerts[1].xyz.y *= scaleToVirtual.y;

	localVerts[1].SetTexCoord( 1.0f, 1.0f );
	localVerts[1].SetColor( PackColor( idVec4() ) );
	localVerts[1].ClearColor2();

	localVerts[2].Clear();
	localVerts[2].xyz[0] = ( _clipRects.x + _clipRects.w );
	localVerts[2].xyz[1] = ( _clipRects.y + _clipRects.h );
	localVerts[2].xyz[2] = 0.0f;
	localVerts[2].xyz -= origin;
	localVerts[2].xyz *= mat;
	localVerts[2].xyz += origin;
	localVerts[2].xyz.x *= scaleToVirtual.x;
	localVerts[2].xyz.y *= scaleToVirtual.y;
	localVerts[2].SetTexCoord( 1.0f, 0.0f );
	localVerts[2].SetColor( PackColor( idVec4() ) );
	localVerts[2].ClearColor2();

	localVerts[3].Clear();
	localVerts[3].xyz[0] = _clipRects.x;
	localVerts[3].xyz[1] = ( _clipRects.y + _clipRects.h );
	localVerts[3].xyz[2] = 0.0f;
	localVerts[3].xyz *= mat;
	localVerts[3].xyz += origin;
	localVerts[3].xyz.x *= scaleToVirtual.x;
	localVerts[3].xyz.y *= scaleToVirtual.y;
	localVerts[3].SetTexCoord( 0.0f, 0.0f );
	localVerts[3].SetColor( PackColor( idVec4() ) );
	localVerts[3].ClearColor2();

	uint64_t glState = 0;

	if( _numMasks == 0 )
	{
		// Nothing written to the stencil buffer yet. Initially seed it with the first clipping rectangle
		glState = GLS_COLORMASK | GLS_ALPHAMASK | GLS_STENCIL_FUNC_ALWAYS | GLS_STENCIL_MAKE_REF( kRmlStencilRef ) | GLS_STENCIL_MAKE_MASK( kRmlStencilMask );
	}
	else
	{
		// Continually decrement the scissor value as the scissor rect heirarchy gets deeper. Unknown what happens when UI window start overlapping. Could be bad.
		glState = GLS_COLORMASK | GLS_ALPHAMASK | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_KEEP | GLS_STENCIL_OP_PASS_DECR;
	}

	idDrawVert* maskVerts = tr_guiModel->AllocTris(
								4,
								quadPicIndexes,
								6,
								reinterpret_cast<const idMaterial*>( _guiSolid ),
								glState,
								STEREO_DEPTH_TYPE_NONE );

	WriteDrawVerts16( maskVerts, localVerts, 4 );
}

void idRmlRender::SetScissorRegion( int x, int y, int width, int height )
{
	_clipRects = idRectangle( x, y, width, height );
}

bool idRmlRender::LoadTexture( Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source )
{
	const idMaterial* material = declManager->FindMaterial( source.c_str() );

	if( !material )
	{
		return false;
	}

	material->SetSort( SS_GUI );

	texture_handle = reinterpret_cast<Rml::TextureHandle>( material );

	// If this material isn't an actual image on disk, then image dimensions from an on-disk image doesn't make sense.
	if( material->GetStage( 0 )->texture.image )
	{
		texture_dimensions.x = material->GetImageWidth();
		texture_dimensions.y = material->GetImageHeight();
	}
	else
	{
		texture_dimensions.x = SCREEN_WIDTH;
		texture_dimensions.y = SCREEN_HEIGHT;
	}

	return true;
}

static void GenerateRmlImage( idImage* image )
{
	// nothing
	return;
}

bool idRmlRender::GenerateTexture( Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions )
{
	idImage* image = globalImages->ImageFromFunction( va( "_rmlImage%d", _texGen ), GenerateRmlImage );

	const idMaterial* material = declManager->FindMaterial( va( "_rmlImage%d", _texGen ) );
	material->SetSort( SS_GUI );

	if( !material )
	{
		return false;
	}

	size_t sz = 4 * source_dimensions.x * source_dimensions.y;
	const byte* mem = ( byte* )Mem_ClearedAlloc( sz, TAG_FONT );
	memcpy( ( void* )mem, source, sz );

	RmlImage rmlImage;
	rmlImage.image = image;
	rmlImage.material = material;
	rmlImage.data = mem;
	rmlImage.dimensions = idVec2( source_dimensions.x, source_dimensions.y );
	rmlImage.referencedOutsideLevelLoad = true; // might need to arrange this in some other way.
	rmlManagerLocal.AddMaterialToReload( &rmlImage );

	texture_handle = reinterpret_cast<Rml::TextureHandle>( material );

	_texGen++;

	return material != nullptr;
}

void idRmlRender::EnableScissorRegion( bool enable )
{
	_enableScissor = enable;

	if( _clipRects.w <= 0 || _clipRects.h <= 0 )
	{
		return;
	}

	if( !_enableScissor )
	{
		return;
	}

	_numMasks++;

	RenderClipMask();
}

void idRmlRender::PreRender()
{
	_numMasks = 0;
}

void idRmlRender::PostRender()
{
}

void idRmlRender::DrawCursor( int x, int y, int w, int h )
{
	const idVec2 scaleToVirtual( ( float )renderSystem->GetVirtualWidth() / renderSystem->GetWidth(),
								 ( float )renderSystem->GetVirtualHeight() / renderSystem->GetHeight() );
	idVec2 bounds;
	bounds.x = w * scaleToVirtual.x;
	bounds.y = h * scaleToVirtual.y;
	float sx = x * scaleToVirtual.x;
	float sy = y * scaleToVirtual.y;
	renderSystem->SetGLState( GLS_DEPTHFUNC_LESS | GLS_DEPTHMASK | GLS_STENCIL_FUNC_GEQUAL | GLS_STENCIL_MAKE_REF( kRmlStencilRef ) | GLS_STENCIL_MAKE_MASK( kRmlStencilMask ) );
	rmlDc->PushClipRect( idRectangle( 0, 0, w, h ) );
	rmlDc->DrawCursor( &sx, &sy, 36.0f * scaleToVirtual.x, bounds );
	rmlDc->PopClipRect();
	renderSystem->SetGLState( 0 );
}

void RmlImage::Free()
{
	if( data )
	{
		Mem_Free( ( void* )data );
		data = 0;
	}
}

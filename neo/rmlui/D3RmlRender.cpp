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

#include "precompiled.h"
#pragma hdrstop

#include "D3RmlRender.h"

#include "renderer/GuiModel.h"
#include "renderer/GLState.h"
#include "renderer/Image.h"

#include "rmlui/RmlUserInterfaceLocal.h"

extern RmlUserInterfaceManagerLocal rmlManagerLocal;

extern idDeviceContext* rmlDc;

extern idGuiModel* tr_guiModel;

/*
===============
idRmlRender

Front end rendering for RML
===============
*/

constexpr int kMaxInitialQuads = 1024;
constexpr int kMaxInitialVerts = kMaxInitialQuads * 4;
constexpr int kMaxInitialTris = kMaxInitialQuads * 6;

constexpr uint64_t kRmlStencilRef = 128;
constexpr int kRmlStencilMask = 255;

idCVar rmlShowStencilMasks( "rml_show_stencil_masks", "0", CVAR_INTEGER, "Draw a red rectangles around the stencil masks" );

// Helper function to get the scaling vector for the guis.
inline idVec2 GetScaleToVirtual()
{
	return idVec2( static_cast<float>( renderSystem->GetVirtualWidth() ) / renderSystem->GetWidth(),
				   static_cast<float>( renderSystem->GetVirtualHeight() ) / renderSystem->GetHeight() );
}


idRmlRender::idRmlRender()
	: enableScissor( false )
	, clipRects()
	, cursorImages()
	, guiSolid( nullptr )
	, numMasks( 0 )
	, drawVerts( nullptr )
	, drawTris( nullptr )
	, texGen( 0 )
{
}

idRmlRender::~idRmlRender()
{
	delete[] drawVerts;
	delete[] drawTris;
}

void idRmlRender::Init()
{
	numMasks = 0;

	// Avoid a bunch of allocations by allocating once and reusing
	// these vertices to draw. We might want to just make this more
	// dynamic to save them and reuse them per frame.
	drawVerts = new idDrawVert[kMaxInitialVerts];
	drawTris = new triIndex_t[kMaxInitialTris];

	origin.Zero();
	mat.Identity();

	guiSolid = declManager->FindMaterial( "_white", false );
}

void idRmlRender::RenderGeometry( Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture, const Rml::Vector2f& translation )
{
	if( numVertices > kMaxInitialVerts )
	{
		common->FatalError( "Increase kMaxInitialVerts" );
		return;
	}

	if( numIndices > kMaxInitialTris )
	{
		common->FatalError( "Increase kMaxInitialTris" );
		return;
	}

	for( int i = 0; i < numIndices; i++ )
	{
		drawTris[i] = indices[i];
	}

	const idVec2 scaleToVirtual( GetScaleToVirtual() );

	idVec3 pos;
	for( int i = 0; i < numVertices; i++ )
	{
		const auto& [position, rmlColor, texCoord] = vertices[i];

		pos.x = position.x + translation.x;
		pos.y = position.y + translation.y;
		pos.z = 0;

		// Transform
		pos *= mat;
		pos += origin;

		pos.x *= scaleToVirtual.x;
		pos.y *= scaleToVirtual.y;

		idDrawVert& srcVertex = drawVerts[i];

		srcVertex.xyz = pos;
		srcVertex.SetTexCoord( texCoord.x, texCoord.y );
		idVec4 color( rmlColor.red, rmlColor.green, rmlColor.blue, rmlColor.alpha );
		color /= 255.0f;
		srcVertex.SetColor( PackColor( color ) );
		srcVertex.ClearColor2();
	}

	const auto* material = reinterpret_cast<const idMaterial*>( texture );

	if( !material )
	{
		material = guiSolid;
	}

	material->SetSort( SS_GUI );

	idDrawVert* verts = tr_guiModel->AllocTris( numVertices, drawTris, numIndices, material, GenerateGlState(), STEREO_DEPTH_TYPE_NONE );

	if( verts )
	{
		WriteDrawVerts16( verts, drawVerts, numVertices );
	}
}

void idRmlRender::SetTransform( const Rml::Matrix4f* transform )
{
	origin.Zero();
	mat.Identity();

	if( !transform )
	{
		return;
	}

	const auto rmlVec1 = transform->GetColumn( 0 );
	const auto rmlVec2 = transform->GetColumn( 1 );
	const auto rmlVec3 = transform->GetColumn( 2 );
	const auto rmlVec4 = transform->GetColumn( 3 );

	origin.Set( rmlVec4.x, rmlVec4.y, rmlVec4.z );

	const idVec3 vecX( rmlVec1.x, rmlVec1.y, rmlVec1.z );
	const idVec3 vecY( rmlVec2.x, rmlVec2.y, rmlVec2.z );
	const idVec3 vecZ( rmlVec3.x, rmlVec3.y, rmlVec3.z );

	mat = idMat3( vecX, vecY, vecZ );
}

static triIndex_t quadPicIndexes[6] = { 3, 0, 2, 2, 0, 1 };

bool idRmlRender::LoadTexture( Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, const Rml::String& source )
{
	const idMaterial* material = declManager->FindMaterial( source.c_str() );

	if( !material )
	{
		return false;
	}

	textureHandle = reinterpret_cast<Rml::TextureHandle>( material );

	// If this material isn't an actual image on disk, then image dimensions from an on-disk image doesn't make sense.
	idImage* image = material->GetStage( 0 )->texture.image;
	if( image )
	{
		textureDimensions.x = material->GetImageWidth();
		textureDimensions.y = material->GetImageHeight();

		if( textureDimensions.x == 0 || textureDimensions.y == 0 )
		{
			image->FinalizeImage( false, nullptr );
			textureDimensions.x = material->GetImageWidth();
			textureDimensions.y = material->GetImageHeight();
		}
	}
	else
	{
		textureDimensions.x = SCREEN_WIDTH;
		textureDimensions.y = SCREEN_HEIGHT;
	}

	return true;
}

void GenerateRmlImage( idImage* image, nvrhi::ICommandList* commandList )
{
	const idStr name = image->GetName( );
	const int hash = name.FileNameHash( );

	for( int i = globalImages->deferredImageHash.First( hash ); i != -1; i = globalImages->deferredImageHash.Next( i ) )
	{
		idDeferredImage* deferredImage = globalImages->deferredImages[i];
		if( name.Icmp( deferredImage->name ) == 0 )
		{
			image->GenerateImage( deferredImage->pic,
								  deferredImage->width,
								  deferredImage->height,
								  deferredImage->textureFilter,
								  deferredImage->textureRepeat,
								  deferredImage->textureUsage,
								  commandList );
			return;
		}
	}
}

bool idRmlRender::GenerateTexture( Rml::TextureHandle& textureHandle, const Rml::byte* source, const Rml::Vector2i& sourceDimensions )
{
	const char* imgName = va( "_rmlImage%d", texGen );
	idImage* image = globalImages->ImageFromFunction( imgName, GenerateRmlImage );

	const idMaterial* material = declManager->FindMaterial( imgName );

	material->SetSort( SS_GUI );

	if( !material )
	{
		return false;
	}

	idDeferredImage* img = globalImages->AllocDeferredImage( imgName );
	img->pic = static_cast<byte*>( Mem_Alloc( 4 * sourceDimensions.x * sourceDimensions.y, TAG_IMAGE ) );
	memcpy( img->pic, source, 4 * sourceDimensions.x * sourceDimensions.y );
	img->width = sourceDimensions.x;
	img->height = sourceDimensions.y;
	img->textureFilter = TF_NEAREST;
	img->textureRepeat = TR_REPEAT;
	img->textureUsage = TD_LOOKUP_TABLE_RGBA;

	textureHandle = reinterpret_cast<Rml::TextureHandle>( material );

	texGen++;

	return material != nullptr;
}

uint64 idRmlRender::GenerateGlState() const
{
	// Don't write to the depth mask.
	uint64 glState( 0 );

	// Stencil
	if( enableScissor )
	{
		// Only draw to the backbuffer if the stencil buffer matches the existing stencil buffer value. This is good for masking.
		glState |= GLS_STENCIL_FUNC_EQUAL | GLS_STENCIL_MAKE_REF( 128 + numMasks ) | GLS_STENCIL_MAKE_MASK( 255 );
	}

	return glState;
}

Rml::CompiledGeometryHandle idRmlRender::CompileGeometry( Rml::Vertex* vertices, int numVertices, int* indices, int numIndices, Rml::TextureHandle texture )
{
	return static_cast<Rml::CompiledGeometryHandle>( 0 );
}

void idRmlRender::RenderCompiledGeometry( Rml::CompiledGeometryHandle geometry, const Rml::Vector2f& translation )
{
	return;
}

void idRmlRender::ReleaseCompiledGeometry( Rml::CompiledGeometryHandle geometry )
{
	return;
}

/*
===============

Clip Masking/Scissor testing.

===============
*/

void idRmlRender::EnableScissorRegion( bool enable )
{
	enableScissor = enable;
}

void idRmlRender::SetScissorRegion( int x, int y, int width, int height )
{
	clipRects = idRectangle( x, y, width, height );
	numMasks++;
	RenderClipMask();
}

void idRmlRender::RenderClipMask()
{
	//return;
	// Usually, scissor regions are handled  with actual scissor render commands.
	// We're using stencil masks to do the same thing because it works in worldspace a
	// lot better than screen space scissor rects.
	const idVec2 scaleToVirtual( GetScaleToVirtual() );

	ALIGNTYPE16 idDrawVert localVerts[4];

	localVerts[0].Clear();
	localVerts[0].xyz[0] = clipRects.x;
	localVerts[0].xyz[1] = clipRects.y;
	localVerts[0].xyz[2] = 0.0f;

	localVerts[0].xyz *= mat;
	localVerts[0].xyz += origin;

	localVerts[0].xyz.x *= scaleToVirtual.x;
	localVerts[0].xyz.y *= scaleToVirtual.y;

	localVerts[0].SetTexCoord( 0.0f, 1.0f );
	localVerts[0].SetColor( PackColor( idVec4() ) );
	localVerts[0].ClearColor2();

	localVerts[1].Clear();
	localVerts[1].xyz[0] = clipRects.x + clipRects.w;
	localVerts[1].xyz[1] = clipRects.y;
	localVerts[1].xyz[2] = 0.0f;

	localVerts[1].xyz *= mat;
	localVerts[1].xyz += origin;

	localVerts[1].xyz.x *= scaleToVirtual.x;
	localVerts[1].xyz.y *= scaleToVirtual.y;

	localVerts[1].SetTexCoord( 1.0f, 1.0f );
	localVerts[1].SetColor( PackColor( idVec4() ) );
	localVerts[1].ClearColor2();

	localVerts[2].Clear();
	localVerts[2].xyz[0] = ( clipRects.x + clipRects.w );
	localVerts[2].xyz[1] = ( clipRects.y + clipRects.h );
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
	localVerts[3].xyz[0] = clipRects.x;
	localVerts[3].xyz[1] = ( clipRects.y + clipRects.h );
	localVerts[3].xyz[2] = 0.0f;
	localVerts[3].xyz *= mat;
	localVerts[3].xyz += origin;
	localVerts[3].xyz.x *= scaleToVirtual.x;
	localVerts[3].xyz.y *= scaleToVirtual.y;
	localVerts[3].SetTexCoord( 0.0f, 0.0f );
	localVerts[3].SetColor( PackColor( idVec4() ) );
	localVerts[3].ClearColor2();

	// Render only to the stencil buffer. Replace the stencil value with the stencil ref + the number of masks.
	uint64_t glState = GLS_OVERRIDE
					   | GLS_DEPTHFUNC_LESS
					   | GLS_COLORMASK
					   | GLS_ALPHAMASK
					   | GLS_STENCIL_OP_PASS_REPLACE
					   | GLS_STENCIL_FUNC_ALWAYS
					   | GLS_STENCIL_MAKE_REF( kRmlStencilRef + numMasks ) | GLS_STENCIL_MAKE_MASK( kRmlStencilMask );

	idDrawVert* maskVerts = tr_guiModel->AllocTris( 4, quadPicIndexes, 6, guiSolid, glState, STEREO_DEPTH_TYPE_NONE );

	WriteDrawVerts16( maskVerts, localVerts, 4 );

	// TODO(Stephen): Make render debug masks configurable.
	if( rmlShowStencilMasks.GetInteger() > 0 )
	{
		DrawRect( clipRects, idVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
	}
}

void idRmlRender::PreRender()
{
	numMasks = 0;
}

void idRmlRender::PostRender()
{
	( void )0;
}

void idRmlRender::DrawCursor( int x, int y, int w, int h )
{
	const idVec2 scaleToVirtual( GetScaleToVirtual() );

	idVec2 bounds;
	bounds.x = w * scaleToVirtual.x;
	bounds.y = h * scaleToVirtual.y;
	float sx = x * scaleToVirtual.x;
	float sy = y * scaleToVirtual.y;
	rmlDc->PushClipRect( idRectangle( 0, 0, w, h ) );
	rmlDc->DrawCursor( &sx, &sy, 36.0f * scaleToVirtual.x, bounds );
	rmlDc->PopClipRect();
	renderSystem->SetGLState( 0 );
}

void idRmlRender::DrawRect( const idRectangle& rect, const idVec4& color )
{
	const idVec2 scaleToVirtual( GetScaleToVirtual() );

	renderSystem->SetColor( color );

	float x = rect.x * scaleToVirtual.x;
	float y = rect.y * scaleToVirtual.y;
	float w = fabs( rect.w ) * scaleToVirtual.x;
	float h = fabs( rect.h ) * scaleToVirtual.y;

	float size = 1;

	renderSystem->DrawStretchPic( x, y, size, h, 0, 0, 0, 0, guiSolid );
	renderSystem->DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, guiSolid );
	renderSystem->DrawStretchPic( x, y, w, size, 0, 0, 0, 0, guiSolid );
	renderSystem->DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, guiSolid );
}

void RmlImage::Free()
{
	if( data )
	{
		Mem_Free( ( void* )data );
		data = 0;
	}
}

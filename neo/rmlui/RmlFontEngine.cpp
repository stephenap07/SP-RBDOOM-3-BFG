/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

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

#include <rmlui/RmlFontEngine.h>

#include "renderer/FontManager.h"
#include "renderer/RenderSystem.h"

#include "RmlUi/Core/FontEngineInterface.h"
#include "RmlUi/Core/FontEffect.h"

RmlFontEngine::RmlFontEngine()
	: fontManager( nullptr )
{
}

RmlFontEngine::~RmlFontEngine()
{
	ReleaseFontResources();
}

void RmlFontEngine::Init()
{
	fontManager = renderSystem->GetFontManager();
	const idMaterial* mat = renderSystem->GetTextBufferManager()->fontMaterial();
	texture.Set( mat->GetName(), mat->GetName() );
}

bool RmlFontEngine::LoadFontFace( const Rml::String& fileName, bool fallbackFace, Rml::Style::FontWeight weight )
{
	const TrueTypeHandle ttf = renderSystem->RegisterFontFace( fileName.c_str() );
	fontFaces.AddUnique( ttf );
	return ttf.id != kInvalidHandle;
}

bool RmlFontEngine::LoadFontFace( const byte* data, int dataSize, const Rml::String& family, Rml::Style::FontStyle style, Rml::Style::FontWeight weight, bool fallbackFace )
{
	return false;
}

Rml::FontFaceHandle RmlFontEngine::GetFontFaceHandle( const Rml::String& family, const Rml::Style::FontStyle style, Rml::Style::FontWeight weight, int size )
{
	FontStyle fontStyle = FONT_STYLE_NORMAL;
	if( style == Rml::Style::FontStyle::Italic )
	{
		fontStyle = FONT_STYLE_ITALIC;
	}

	FontWeight fontWeight = FontWeight::Normal;
	if( weight == Rml::Style::FontWeight::Bold )
	{
		fontWeight = FontWeight::Bold;
	}

	const FontHandle handle{ renderSystem->RegisterFont2( family.c_str(), size, fontStyle, fontWeight ) };

	if( handle.id == kInvalidHandle )
	{
		return reinterpret_cast<Rml::FontFaceHandle>( nullptr );
	}

	fontHandles.AddUnique( handle );
	return static_cast<Rml::FontFaceHandle>( handle.id ) + 1;
}

Rml::FontEffectsHandle RmlFontEngine::PrepareFontEffects( Rml::FontFaceHandle handle, const Rml::FontEffectList& rmlFontEffects )
{
	int nextNull = fontEffects.FindNull();

	if( nextNull == -1 )
	{
		return fontEffects.Append( new FontEffect() );
	}

	fontEffects[nextNull] = new FontEffect();
	return nextNull;
}

int RmlFontEngine::GetSize( const Rml::FontFaceHandle handle )
{
	FontHandle fontHandle{ static_cast<uint16_t>( handle - 1 ) };
	return fontManager->getFontInfo( fontHandle ).pixelSize;
}

int RmlFontEngine::GetXHeight( Rml::FontFaceHandle handle )
{
	const auto textBufferManager = renderSystem->GetTextBufferManager();
	const TextBufferHandle textHandle = textBufferManager->createTextBuffer( 1, BufferType::Transient );
	FontHandle fontHandle{ static_cast<uint16_t>( handle - 1 ) };
	textBufferManager->appendText( textHandle, fontHandle, "x" );
	const int height = textBufferManager->getRectangle( textHandle ).height;
	textBufferManager->destroyTextBuffer( textHandle );
	return height;
}

int RmlFontEngine::GetLineHeight( Rml::FontFaceHandle handle )
{
	FontHandle fontHandle{ static_cast<uint16_t>( handle - 1 ) };
	const auto& info = fontManager->getFontInfo( fontHandle );
	return info.ascender - info.descender;
}

int RmlFontEngine::GetBaseline( Rml::FontFaceHandle handle )
{
	FontHandle fontHandle{ static_cast<uint16_t>( handle - 1 ) };
	return fontManager->getFontInfo( fontHandle ).ascender;
}

float RmlFontEngine::GetUnderline( Rml::FontFaceHandle handle, float& thickness )
{
	FontHandle fontHandle{ static_cast<uint16_t>( handle - 1 ) };
	return fontManager->getFontInfo( fontHandle ).underlinePosition;
}

int RmlFontEngine::GetStringWidth( Rml::FontFaceHandle handle, const Rml::String& string, Rml::Character priorCharacter )
{
	const auto textBufferManager = renderSystem->GetTextBufferManager();
	const TextBufferHandle textHandle = textBufferManager->createTextBuffer( 1, BufferType::Transient );
	FontHandle fontHandle{ static_cast<uint16_t>( handle ) - 1 };
	textBufferManager->appendText( textHandle, fontHandle, string.c_str() );
	const int width = textBufferManager->getRectangle( textHandle ).width;
	textBufferManager->destroyTextBuffer( textHandle );
	return width;
}

int RmlFontEngine::GenerateString( Rml::FontFaceHandle faceHandle, Rml::FontEffectsHandle fontEffectsHandle, const Rml::String& string, const Rml::Vector2f& position, const Rml::Colourb& colour, float opacity, Rml::GeometryList& geometry )
{
	const auto textBufferManager = renderSystem->GetTextBufferManager();
	const TextBufferHandle textHandle = textBufferManager->createTextBuffer( 1, BufferType::Transient );

	textBufferManager->setPenPosition( textHandle, position.x, position.y );
	textBufferManager->setTextColor( textHandle, VectorUtil::Vec4ToColorInt( idVec4( colour.red, colour.green, colour.blue, opacity * colour.alpha ) / 255.f ) );
	FontHandle fontHandle{ static_cast<uint16_t>( faceHandle - 1 ) };
	textBufferManager->appendText( textHandle, fontHandle, string.c_str() );

	geometry.emplace_back();
	Rml::Geometry& geo = geometry.back();

	const int vertexCount = textBufferManager->vertexCount( textHandle );
	geo.GetVertices().reserve( vertexCount );
	const idDrawVert* verts = textBufferManager->getVertices( textHandle );

	for( int i = 0; i < vertexCount; i++ )
	{
		auto& vert = verts[ i ].xyz;
		Rml::Vertex vertex;
		vertex.position = { vert.x, vert.y };
		idVec4 color;
		UnpackColor( verts[ i ].GetColor(), color );
		vertex.colour = { idMath::Ftob( color.x * 255.f ),
						  idMath::Ftob( color.y * 255.f ),
						  idMath::Ftob( color.z * 255.f ),
						  idMath::Ftob( color.w * 255.f )
						};
		vertex.tex_coord = { verts[i].GetTexCoordS(), verts[i].GetTexCoordT() };
		geo.GetVertices().push_back( vertex );
	}

	const int indexCount = textBufferManager->indexCount( textHandle );
	geo.GetIndices().reserve( indexCount );
	const uint16_t* indexes = textBufferManager->getIndexes( textHandle );

	for( int i = 0; i < indexCount; i++ )
	{
		geo.GetIndices().emplace_back( indexes[ i ] );
	}

	geo.SetTexture( &texture );

	const int width = textBufferManager->getRectangle( textHandle ).width;
	textBufferManager->destroyTextBuffer( textHandle );

	return width;
}

int RmlFontEngine::GetVersion( Rml::FontFaceHandle handle )
{
	return 0;
}

void RmlFontEngine::ReleaseFontResources()
{
	for( const auto font : fontHandles )
	{
		renderSystem->FreeFont( font );
	}
	fontHandles.Clear();

	for( const auto fontFace : fontFaces )
	{
		renderSystem->FreeFontFace( fontFace );
	}
	fontFaces.Clear();
}

/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2016 Robert Beckebans

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

/*
================================================================================================
Contains the Image implementation for OpenGL.
================================================================================================
*/

#include "../RenderCommon.h"

#include "sys/DeviceManager.h"

extern DeviceManager* deviceManager;

/*
====================
idImage::idImage
====================
*/
idImage::idImage( const char* name ) : imgName( name )
{
	texnum = TEXTURE_NOT_LOADED;
	generatorFunction = NULL;
	filter = TF_DEFAULT;
	repeat = TR_REPEAT;
	usage = TD_DEFAULT;
	cubeFiles = CF_2D;
	cubeMapSize = 0;

	referencedOutsideLevelLoad = false;
	levelLoadReferenced = false;
	defaulted = false;
	sourceFileTime = FILE_NOT_FOUND_TIMESTAMP;
	binaryFileTime = FILE_NOT_FOUND_TIMESTAMP;
	refCount = 0;
}

/*
====================
idImage::~idImage
====================
*/
idImage::~idImage()
{
	PurgeImage();
}

/*
====================
idImage::IsLoaded
====================
*/
bool idImage::IsLoaded() const
{
	return texnum != TEXTURE_NOT_LOADED;
}

/*
==============
Bind

Automatically enables 2D mapping or cube mapping if needed
==============
*/
void idImage::Bind()
{
	RENDERLOG_PRINTF( "idImage::Bind( %s )\n", GetName() );

	// load the image if necessary (FIXME: not SMP safe!)
	// RB: don't try again if last time failed
	if( !IsLoaded( ) && !defaulted )
	{
		// load the image on demand here, which isn't our normal game operating mode
		ActuallyLoadImage( true );
	}


}

/*
====================
CopyFramebuffer
====================
*/
void idImage::CopyFramebuffer( int x, int y, int imageWidth, int imageHeight )
{
	tr.backend.pc.c_copyFrameBuffer++;
}

/*
====================
CopyDepthbuffer
====================
*/
void idImage::CopyDepthbuffer( int x, int y, int imageWidth, int imageHeight )
{
	tr.backend.pc.c_copyFrameBuffer++;
}

/*
========================
idImage::SubImageUpload
========================
*/
void idImage::SubImageUpload( int mipLevel, int x, int y, int z, int width, int height, const void* pic, int pixelPitch )
{
	assert( x >= 0 && y >= 0 && mipLevel >= 0 && width >= 0 && height >= 0 && mipLevel < opts.numLevels );
}

/*
========================
idImage::SetSamplerState
========================
*/
void idImage::SetSamplerState( textureFilter_t tf, textureRepeat_t tr )
{
}

/*
========================
idImage::SetPixel
========================
*/
void idImage::SetPixel( int mipLevel, int x, int y, const void* data, int dataSize )
{
	SubImageUpload( mipLevel, x, y, 0, 1, 1, data );
}

/*
========================
idImage::SetTexParameters
========================
*/
void idImage::SetTexParameters()
{
}

/*
========================
idImage::AllocImage

Every image will pass through this function. Allocates all the necessary MipMap levels for the
Image, but doesn't put anything in them.

This should not be done during normal game-play, if you can avoid it.
========================
*/
void idImage::AllocImage()
{
	PurgeImage();

	nvrhi::Format format;

	switch( opts.format )
	{
		case FMT_RGBA8:
			format = nvrhi::Format::RGBA8_UINT;
			break;

		case FMT_XRGB8:
			format = nvrhi::Format::X32G8_UINT;
			break;

		case FMT_RGB565:
			format = nvrhi::Format::B5G6R5_UNORM;
			break;

		case FMT_ALPHA:
			format = nvrhi::Format::R8_UINT;
			break;

		case FMT_L8A8:
			format = nvrhi::Format::RG8_UINT;
			break;

		case FMT_LUM8:
			format = nvrhi::Format::R8_UINT;
			break;

		case FMT_INT8:
			format = nvrhi::Format::R8_UINT;
			break;

		case FMT_DXT1:
			format = nvrhi::Format::BC1_UNORM_SRGB;
			break;

		case FMT_DXT5:
			format = nvrhi::Format::BC3_UNORM_SRGB;
			break;

		case FMT_DEPTH:
			format = nvrhi::Format::D32;
			break;

		case FMT_DEPTH_STENCIL:
			format = nvrhi::Format::D24S8;
			break;

		case FMT_SHADOW_ARRAY:
			format = nvrhi::Format::D32;
			break;

		case FMT_RG16F:
			format = nvrhi::Format::RG16_FLOAT;
			break;

		case FMT_RGBA16F:
			format = nvrhi::Format::RGBA16_FLOAT;
			break;

		case FMT_RGBA32F:
			format = nvrhi::Format::RGBA32_FLOAT;
			break;

		case FMT_R32F:
			format = nvrhi::Format::R32_FLOAT;
			break;

		case FMT_X16:
			//internalFormat = GL_INTENSITY16;
			//dataFormat = GL_LUMINANCE;
			//dataType = GL_UNSIGNED_SHORT;
			//format = nvrhi::Format::Lum
			break;
		case FMT_Y16_X16:
			//internalFormat = GL_LUMINANCE16_ALPHA16;
			//dataFormat = GL_LUMINANCE_ALPHA;
			//dataType = GL_UNSIGNED_SHORT;
			break;

		// see http://what-when-how.com/Tutorial/topic-615ll9ug/Praise-for-OpenGL-ES-30-Programming-Guide-291.html
		case FMT_R11G11B10F:
			//internalFormat = GL_R11F_G11F_B10F;
			//dataFormat = GL_RGB;
			//dataType = GL_UNSIGNED_INT_10F_11F_11F_REV;
			format = nvrhi::Format::R11G11B10_FLOAT;
			break;

		default:
			idLib::Error( "Unhandled image format %d in %s\n", opts.format, GetName() );
	}

	// if we don't have a rendering context, just return after we
	// have filled in the parms.  We must have the values set, or
	// an image match from a shader before OpenGL starts would miss
	// the generated texture
	if( !tr.IsInitialized() )
	{
		return;
	}

	auto textureDesc = nvrhi::TextureDesc( )
		.setDimension( nvrhi::TextureDimension::Texture2D )
		.setWidth( opts.width )
		.setHeight( opts.height )
		.setClearValue( nvrhi::Color( 0.f ) )
		.setKeepInitialState( true )
		.setFormat( format )
		.setMipLevels( 1 );

	if( opts.textureType == TT_2D )
	{
		textureDesc.setDimension( nvrhi::TextureDimension::Texture2D );
	}
	else if( opts.textureType == TT_CUBIC )
	{
		textureDesc.setDimension( nvrhi::TextureDimension::TextureCube );
		textureDesc.setArraySize( 6 );
	}
	// RB begin
	else if( opts.textureType == TT_2D_ARRAY )
	{
		textureDesc.setDimension( nvrhi::TextureDimension::Texture2DArray );
		textureDesc.setArraySize( 6 );
	}
	else if( opts.textureType == TT_2D_MULTISAMPLE )
	{
		textureDesc.setDimension( nvrhi::TextureDimension::Texture2DMS );
		textureDesc.setArraySize( 1 ); 
	}

	// Upload the texture to the GPU
}

/*
========================
idImage::PurgeImage
========================
*/
void idImage::PurgeImage()
{
}

/*
========================
idImage::Resize
========================
*/
void idImage::Resize( int width, int height )
{
	if( opts.width == width && opts.height == height )
	{
		return;
	}
	opts.width = width;
	opts.height = height;
	AllocImage();
}

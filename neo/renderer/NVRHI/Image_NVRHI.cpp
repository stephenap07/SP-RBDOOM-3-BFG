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
	texture.Reset( );
	generatorFunction = NULL;
	filter = TF_DEFAULT;
	repeat = TR_REPEAT;
	usage = TD_DEFAULT;
	cubeFiles = CF_2D;
	cubeMapSize = 0;
	isLoaded = false;

	referencedOutsideLevelLoad = false;
	levelLoadReferenced = false;
	defaulted = false;
	sourceFileTime = FILE_NOT_FOUND_TIMESTAMP;
	binaryFileTime = FILE_NOT_FOUND_TIMESTAMP;
	refCount = 0;

	DeferredLoadImage( );
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
	return isLoaded;
}

void idImage::CreateSampler( )
{
	auto samplerDesc = nvrhi::SamplerDesc( )
		.setAllFilters( false )
		.setAllAddressModes( nvrhi::SamplerAddressMode::Clamp )
		.setMaxAnisotropy( 1.0f );

	if( opts.format == FMT_DEPTH )
	{
		samplerDesc.setReductionType( nvrhi::SamplerReductionType::Comparison );
	}

	switch( filter )
	{
	case TF_DEFAULT:
		samplerDesc.minFilter = true;
		samplerDesc.setAllFilters( true )
			.setMaxAnisotropy( r_maxAnisotropicFiltering.GetInteger( ) );

		break;

	case TF_LINEAR:
		samplerDesc.setAllFilters( true );
		break;

	case TF_NEAREST:
		samplerDesc.setAllFilters( false );
		break;

		// RB:
	case TF_NEAREST_MIPMAP:
		samplerDesc.setAllFilters( false );
		break;

	default:
		idLib::FatalError( "idImage::CreateSampler: unrecognized texture filter %d", filter );
	}

	switch( repeat )
	{
	case TR_REPEAT:
		samplerDesc.setAddressU( nvrhi::SamplerAddressMode::Repeat )
			.setAddressV( nvrhi::SamplerAddressMode::Repeat )
			.setAddressW( nvrhi::SamplerAddressMode::Repeat );
		break;

	case TR_CLAMP:
		samplerDesc.setAddressU( nvrhi::SamplerAddressMode::ClampToEdge )
			.setAddressV( nvrhi::SamplerAddressMode::ClampToEdge )
			.setAddressW( nvrhi::SamplerAddressMode::ClampToEdge );
		break;

	case TR_CLAMP_TO_ZERO_ALPHA:
		samplerDesc.setBorderColor( nvrhi::Color(0.f, 0.f, 0.f, 0.f) )
			.setAddressU( nvrhi::SamplerAddressMode::ClampToBorder )
			.setAddressV( nvrhi::SamplerAddressMode::ClampToBorder )
			.setAddressW( nvrhi::SamplerAddressMode::ClampToBorder );
		break;

	case TR_CLAMP_TO_ZERO:
		samplerDesc.setBorderColor( nvrhi::Color( 0.f, 0.f, 0.f, 1.f ) )
			.setAddressU( nvrhi::SamplerAddressMode::ClampToBorder )
			.setAddressV( nvrhi::SamplerAddressMode::ClampToBorder )
			.setAddressW( nvrhi::SamplerAddressMode::ClampToBorder );
		break;
	default:
		idLib::FatalError( "idImage::CreateSampler: unrecognized texture repeat mode %d", repeat );
	}

	sampler = deviceManager->GetDevice( )->createSampler( samplerDesc );
}

/*
==============
Bind

Automatically enables 2D mapping or cube mapping if needed
==============
*/
void idImage::Bind( )
{
	RENDERLOG_PRINTF( "idImage::Bind( %s )\n", GetName() );

	tr.backend.SetCurrentImage( this );
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
void idImage::SubImageUpload( int mipLevel, int x, int y, int z, int width, int height, const void* pic, nvrhi::ICommandList* commandList, int pixelPitch )
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
idImage::SetTexParameters
========================
*/
void idImage::SetTexParameters()
{
	int target = GL_TEXTURE_2D;
	switch( opts.textureType )
	{
	case TT_2D:
		target = GL_TEXTURE_2D;
		break;
	case TT_CUBIC:
		target = GL_TEXTURE_CUBE_MAP;
		break;
		// RB begin
	case TT_2D_ARRAY:
		target = GL_TEXTURE_2D_ARRAY;
		break;
	case TT_2D_MULTISAMPLE:
		//target = GL_TEXTURE_2D_MULTISAMPLE;
		//break;
		// no texture parameters for MSAA FBO textures
		return;
		// RB end
	default:
		idLib::FatalError( "%s: bad texture type %d", GetName( ), opts.textureType );
		return;
	}

	// ALPHA, LUMINANCE, LUMINANCE_ALPHA, and INTENSITY have been removed
	// in OpenGL 3.2. In order to mimic those modes, we use the swizzle operators
	if( opts.colorFormat == CFM_GREEN_ALPHA )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	}
	else if( opts.format == FMT_LUM8 )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_ONE );
	}
	else if( opts.format == FMT_L8A8 )//|| opts.format == FMT_RG16F )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	}
	else if( opts.format == FMT_ALPHA )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	}
	else if( opts.format == FMT_INT8 )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	}
	else
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_GREEN );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_BLUE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_ALPHA );
	}

	switch( filter )
	{
	case TF_DEFAULT:
		if( r_useTrilinearFiltering.GetBool( ) )
		{
			glTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		}
		else
		{
			glTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		}
		glTexParameterf( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		break;
	case TF_LINEAR:
		glTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterf( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		break;
	case TF_NEAREST:
	case TF_NEAREST_MIPMAP:
		glTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameterf( target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		break;
	default:
		common->FatalError( "%s: bad texture filter %d", GetName( ), filter );
	}

	if( glConfig.anisotropicFilterAvailable )
	{
		// only do aniso filtering on mip mapped images
		if( filter == TF_DEFAULT )
		{
			int aniso = r_maxAnisotropicFiltering.GetInteger( );
			if( aniso > glConfig.maxTextureAnisotropy )
			{
				aniso = glConfig.maxTextureAnisotropy;
			}
			if( aniso < 0 )
			{
				aniso = 0;
			}
			glTexParameterf( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );
		}
		else
		{
			glTexParameterf( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1 );
		}
	}

	// RB: disabled use of unreliable extension that can make the game look worse but doesn't save any VRAM
	/*
	if( glConfig.textureLODBiasAvailable && ( usage != TD_FONT ) )
	{
		// use a blurring LOD bias in combination with high anisotropy to fix our aliasing grate textures...
		glTexParameterf( target, GL_TEXTURE_LOD_BIAS_EXT, 0.5 ); //r_lodBias.GetFloat() );
	}
	*/
	// RB end

	// set the wrap/clamp modes
	switch( repeat )
	{
	case TR_REPEAT:
		glTexParameterf( target, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( target, GL_TEXTURE_WRAP_T, GL_REPEAT );
		break;
	case TR_CLAMP_TO_ZERO:
	{
		float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glTexParameterfv( target, GL_TEXTURE_BORDER_COLOR, color );
		glTexParameterf( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameterf( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	}
	break;
	case TR_CLAMP_TO_ZERO_ALPHA:
	{
		float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glTexParameterfv( target, GL_TEXTURE_BORDER_COLOR, color );
		glTexParameterf( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameterf( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	}
	break;
	case TR_CLAMP:
		glTexParameterf( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		break;
	default:
		common->FatalError( "%s: bad texture repeat %d", GetName( ), repeat );
	}

	// RB: added shadow compare parameters for shadow map textures
	if( opts.format == FMT_SHADOW_ARRAY )
	{
		//glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
		glTexParameteri( target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	}
}

/*
========================
idImage::AllocImage

Every image will pass through this function. Allocates all the necessary MipMap levels for the
Image, but doesn't put anything in them.

This should not be done during normal game-play, if you can avoid it.
========================
*/
void idImage::AllocImage( )
{
	PurgeImage();

	nvrhi::Format format = nvrhi::Format::RGBA8_UINT;
	int bpp = 4;

	CreateSampler( );

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

	int compressedSize = 0;
	uint originalWidth = opts.width;
	uint originalHeight = opts.height;

	if( IsCompressed( ) )
	{
		originalWidth = ( originalWidth + 3 ) & ~3;
		originalHeight = ( originalHeight + 3 ) & ~3;
	}

	uint scaledWidth = originalWidth;
	uint scaledHeight = originalHeight;

	uint maxTextureSize = 0;

	if( maxTextureSize > 0 &&
			int( std::max( originalWidth, originalHeight ) ) > maxTextureSize &&
			opts.isRenderTarget &&
			opts.textureType == TT_2D )
	{
		if( originalWidth >= originalHeight )
		{
			scaledHeight = originalHeight * maxTextureSize / originalWidth;
			scaledWidth = maxTextureSize;
		}
		else
		{
			scaledWidth = originalWidth * maxTextureSize / originalHeight;
			scaledHeight = maxTextureSize;
		}
	}

	auto textureDesc = nvrhi::TextureDesc( )
					   .setDebugName( GetName() )
					   .setDimension( nvrhi::TextureDimension::Texture2D )
					   .setWidth( scaledWidth )
					   .setHeight( scaledHeight )
					   .setFormat( format )
					   .setMipLevels( opts.numLevels );

	if( opts.isRenderTarget )
	{
		textureDesc.keepInitialState = true;
		textureDesc.isRenderTarget = opts.isRenderTarget;

		if( opts.format != FMT_DEPTH && opts.format != FMT_DEPTH_STENCIL )
		{
			textureDesc.setIsUAV( true );
		}
	}

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

	texture = deviceManager->GetDevice( )->createTexture( textureDesc );

	assert( texture );
}

/*
========================
idImage::PurgeImage
========================
*/
void idImage::PurgeImage()
{
	texture.Reset( );
	sampler.Reset( );
	isLoaded = false;
	defaulted = false;
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

#include "precompiled.h"
#pragma hdrstop

#include "CommonPasses.h"


CommonRenderPasses::CommonRenderPasses( )
	: m_Device( nullptr )
{
}

static bool IsSupportedBlitDimension( nvrhi::TextureDimension dimension )
{
	return dimension == nvrhi::TextureDimension::Texture2D
		   || dimension == nvrhi::TextureDimension::Texture2DArray
		   || dimension == nvrhi::TextureDimension::TextureCube
		   || dimension == nvrhi::TextureDimension::TextureCubeArray;
}

static bool IsTextureArray( nvrhi::TextureDimension dimension )
{
	return dimension == nvrhi::TextureDimension::Texture2DArray
		   || dimension == nvrhi::TextureDimension::TextureCube
		   || dimension == nvrhi::TextureDimension::TextureCubeArray;
}

void CommonRenderPasses::Init( nvrhi::IDevice* device )
{
	m_Device = device;

	auto samplerDesc = nvrhi::SamplerDesc( )
		.setAllFilters( false )
		.setAllAddressModes( nvrhi::SamplerAddressMode::Clamp );
	m_PointClampSampler = m_Device->createSampler( samplerDesc );

	samplerDesc.setAllFilters( true );
	m_LinearClampSampler = m_Device->createSampler( samplerDesc );

	samplerDesc.setAllAddressModes( nvrhi::SamplerAddressMode::Wrap );
	m_LinearWrapSampler = m_Device->createSampler( samplerDesc );

	samplerDesc.setMaxAnisotropy( 16 );
	m_AnisotropicWrapSampler = m_Device->createSampler( samplerDesc );

	{
		unsigned int blackImage = 0xff000000;
		unsigned int grayImage = 0xff808080;
		unsigned int whiteImage = 0xffffffff;

		nvrhi::TextureDesc textureDesc;
		textureDesc.format = nvrhi::Format::RGBA8_UNORM;
		textureDesc.width = 1;
		textureDesc.height = 1;
		textureDesc.mipLevels = 1;

		textureDesc.debugName = "BlackTexture";
		m_BlackTexture = m_Device->createTexture( textureDesc );

		textureDesc.debugName = "GrayTexture";
		m_GrayTexture = m_Device->createTexture( textureDesc );

		textureDesc.debugName = "WhiteTexture";
		m_WhiteTexture = m_Device->createTexture( textureDesc );

		textureDesc.dimension = nvrhi::TextureDimension::TextureCubeArray;
		textureDesc.debugName = "BlackCubeMapArray";
		textureDesc.arraySize = 6;
		m_BlackCubeMapArray = m_Device->createTexture( textureDesc );

		textureDesc.dimension = nvrhi::TextureDimension::Texture2DArray;
		textureDesc.debugName = "BlackTexture2DArray";
		textureDesc.arraySize = 6;
		m_BlackTexture2DArray = m_Device->createTexture( textureDesc );
		textureDesc.debugName = "WhiteTexture2DArray";
		m_WhiteTexture2DArray = m_Device->createTexture( textureDesc );

		// Write the textures using a temporary CL

		nvrhi::CommandListHandle commandList = m_Device->createCommandList( );
		commandList->open( );

		commandList->beginTrackingTextureState( m_BlackTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common );
		commandList->beginTrackingTextureState( m_GrayTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common );
		commandList->beginTrackingTextureState( m_WhiteTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common );
		commandList->beginTrackingTextureState( m_BlackCubeMapArray, nvrhi::AllSubresources, nvrhi::ResourceStates::Common );
		commandList->beginTrackingTextureState( m_BlackTexture2DArray, nvrhi::AllSubresources, nvrhi::ResourceStates::Common );
		commandList->beginTrackingTextureState( m_WhiteTexture2DArray, nvrhi::AllSubresources, nvrhi::ResourceStates::Common );

		commandList->writeTexture( m_BlackTexture, 0, 0, &blackImage, 0 );
		commandList->writeTexture( m_GrayTexture, 0, 0, &grayImage, 0 );
		commandList->writeTexture( m_WhiteTexture, 0, 0, &whiteImage, 0 );

		for( uint32_t arraySlice = 0; arraySlice < 6; arraySlice += 1 )
		{
			commandList->writeTexture( m_BlackTexture2DArray, arraySlice, 0, &blackImage, 0 );
			commandList->writeTexture( m_WhiteTexture2DArray, arraySlice, 0, &whiteImage, 0 );
			commandList->writeTexture( m_BlackCubeMapArray, arraySlice, 0, &blackImage, 0 );
		}

		commandList->setPermanentTextureState( m_BlackTexture, nvrhi::ResourceStates::ShaderResource );
		commandList->setPermanentTextureState( m_GrayTexture, nvrhi::ResourceStates::ShaderResource );
		commandList->setPermanentTextureState( m_WhiteTexture, nvrhi::ResourceStates::ShaderResource );
		commandList->setPermanentTextureState( m_BlackCubeMapArray, nvrhi::ResourceStates::ShaderResource );
		commandList->setPermanentTextureState( m_BlackTexture2DArray, nvrhi::ResourceStates::ShaderResource );
		commandList->setPermanentTextureState( m_WhiteTexture2DArray, nvrhi::ResourceStates::ShaderResource );
		commandList->commitBarriers( );

		commandList->close( );
		m_Device->executeCommandList( commandList );
	}

	{
		nvrhi::BindingLayoutDesc layoutDesc;
		layoutDesc.visibility = nvrhi::ShaderType::All;
		layoutDesc.bindings =
		{
			nvrhi::BindingLayoutItem::PushConstants( 0, sizeof( BlitConstants ) ),
			nvrhi::BindingLayoutItem::Texture_SRV( 0 ),
			nvrhi::BindingLayoutItem::Sampler( 0 )
		};

		m_BlitBindingLayout = m_Device->createBindingLayout( layoutDesc );
	}
}

void CommonRenderPasses::BlitTexture( nvrhi::ICommandList* commandList, const BlitParameters& params, BindingCache* bindingCache )
{
}

void CommonRenderPasses::BlitTexture( nvrhi::ICommandList* commandList, nvrhi::IFramebuffer* targetFramebuffer, nvrhi::ITexture* sourceTexture, BindingCache* bindingCache )
{
	assert( commandList );
	assert( targetFramebuffer );
	assert( sourceTexture );

	BlitParameters params;
	params.targetFramebuffer = targetFramebuffer;
	params.sourceTexture = sourceTexture;
	BlitTexture( commandList, params, bindingCache );
}

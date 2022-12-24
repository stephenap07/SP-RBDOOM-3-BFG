#include "precompiled.h"
#pragma hdrstop

#include "RenderCommon.h"
#include "Image.h"

#include "MaterialBindingCache.h"

MaterialBindingCache::MaterialBindingCache(
	nvrhi::IDevice* device,
	nvrhi::ShaderType shaderType,
	uint32_t registerSpace,
	const std::vector<MaterialResourceBinding>& bindings,
	nvrhi::ISampler* sampler,
	nvrhi::ITexture* fallbackTexture,
	bool trackLiveness )
	: device( device )
	, shaderType( shaderType )
	, bindingDesc( bindings )
	, fallbackTexture( fallbackTexture )
	, sampler( sampler )
	, trackLiveness( trackLiveness )
{
	nvrhi::BindingLayoutDesc layoutDesc;
	layoutDesc.visibility = shaderType;
	layoutDesc.registerSpace = registerSpace;

	for( const auto& item : bindings )
	{
		nvrhi::BindingLayoutItem layoutItem{};
		layoutItem.slot = item.slot;

		switch( item.resource )
		{
			case MaterialResource::ConstantBuffer:
				layoutItem.type = nvrhi::ResourceType::ConstantBuffer;
				break;
			case MaterialResource::DiffuseTexture:
			case MaterialResource::SpecularTexture:
			case MaterialResource::NormalTexture:
			case MaterialResource::EmissiveTexture:
				layoutItem.type = nvrhi::ResourceType::Texture_SRV;
				break;
			case MaterialResource::Sampler:
				layoutItem.type = nvrhi::ResourceType::Sampler;
				break;
			default:
				common->Error( "MaterialBindingCache: unknown MaterialResource value (%d)", item.resource );
				return;
		}

		layoutDesc.bindings.push_back( layoutItem );
	}

	bindingLayout = device->createBindingLayout( layoutDesc );
}

nvrhi::IBindingLayout* ::MaterialBindingCache::GetLayout() const
{
	return bindingLayout;
}

nvrhi::IBindingSet* MaterialBindingCache::GetMaterialBindingSet( const drawSurf_t* drawSurf )
{
	mutex.Lock();

	nvrhi::BindingSetHandle& bindingSet = bindingSets[drawSurf->material];

	if( bindingSet )
	{
		mutex.Unlock();
		return bindingSet;
	}

	bindingSet = CreateMaterialBindingSet( drawSurf );

	mutex.Unlock();
	return bindingSet;
}

void MaterialBindingCache::Clear()
{
	mutex.Lock();
	bindingSets.clear();
	mutex.Unlock();
}

nvrhi::BindingSetItem MaterialBindingCache::GetTextureBindingSetItem( uint32_t slot, const idImage* texture ) const
{
	return nvrhi::BindingSetItem::Texture_SRV( slot, texture ? texture->GetTextureHandle().Get() : fallbackTexture.Get() );
}

nvrhi::BindingSetHandle MaterialBindingCache::CreateMaterialBindingSet( const drawSurf_t* drawSurf )
{
	nvrhi::BindingSetDesc bindingSetDesc;
	bindingSetDesc.trackLiveness = trackLiveness;

	idImage* diffuse = nullptr;
	idImage* normal = nullptr;
	idImage* specular = nullptr;
	idImage* emissive = nullptr;
	const idMaterial* material = drawSurf->material;
	idUniformBuffer cb;

	for( int stage = 0; stage < material->GetNumStages(); stage++ )
	{
		const shaderStage_t* pStage = material->GetStage( stage );
		switch( pStage->lighting )
		{
			case SL_AMBIENT:
				emissive = pStage->texture.image;
				break;
			case SL_BUMP:
				normal = pStage->texture.image;
				break;
			case SL_DIFFUSE:
				diffuse = pStage->texture.image;
				break;
			case SL_SPECULAR:
				specular = pStage->texture.image;
				break;
		}
	}

	for( const auto& item : bindingDesc )
	{
		nvrhi::BindingSetItem setItem;

		switch( item.resource )
		{
			case MaterialResource::ConstantBuffer:
			{
				//vertexCache.GetMaterialBuffer( drawSurf->matRegisterCache, &cb );
				//nvrhi::IBuffer* buffer = cb.GetAPIObject();
				//if( buffer )
				{
					setItem = nvrhi::BindingSetItem::ConstantBuffer(
								  item.slot,
								  vertexCache.staticData.materialBuffer->GetBuffer() );
				}
				break;
			}

			case MaterialResource::Sampler:
				setItem = nvrhi::BindingSetItem::Sampler( item.slot, sampler );
				break;

			case MaterialResource::DiffuseTexture:
				setItem = GetTextureBindingSetItem( item.slot, diffuse );
				break;

			case MaterialResource::SpecularTexture:
				setItem = GetTextureBindingSetItem( item.slot, specular );
				break;

			case MaterialResource::NormalTexture:
				setItem = GetTextureBindingSetItem( item.slot, normal );
				break;

			case MaterialResource::EmissiveTexture:
				setItem = GetTextureBindingSetItem( item.slot, emissive );
				break;

			default:
				common->Error( "MaterialBindingCache: unknown MaterialResource value (%d)", item.resource );
				return nullptr;
		}

		bindingSetDesc.bindings.push_back( setItem );
	}

	return device->createBindingSet( bindingSetDesc, bindingLayout );
}

#pragma once


enum class MaterialResource
{
	ConstantBuffer,
	Sampler,
	DiffuseTexture,
	SpecularTexture,
	NormalTexture,
	EmissiveTexture,
	OcclusionTexture,
	TransmissionTexture
};

struct MaterialResourceBinding
{
	MaterialResource resource;
	uint32_t slot; // type depends on resource
};

class MaterialBindingCache
{
public:
	MaterialBindingCache(
		nvrhi::IDevice* device,
		nvrhi::ShaderType shaderType,
		uint32_t registerSpace,
		const std::vector<MaterialResourceBinding>& bindings,
		nvrhi::ISampler* sampler,
		nvrhi::ITexture* fallbackTexture,
		bool trackLiveness = true );

	nvrhi::IBindingLayout*  GetLayout() const;
	nvrhi::IBindingSet*     GetMaterialBindingSet( const drawSurf_t* drawSurf );
	void                    Clear();

private:

	nvrhi::BindingSetHandle CreateMaterialBindingSet( const drawSurf_t* drawSurf );
	nvrhi::BindingSetItem   GetTextureBindingSetItem( uint32_t slot, const idImage* texture ) const;

	nvrhi::DeviceHandle                                             device;
	nvrhi::BindingLayoutHandle                                      bindingLayout;
	std::unordered_map<const idMaterial*, nvrhi::BindingSetHandle>  bindingSets;
	nvrhi::ShaderType                                               shaderType;
	std::vector<MaterialResourceBinding>                            bindingDesc;
	nvrhi::TextureHandle                                            fallbackTexture;
	nvrhi::SamplerHandle                                            sampler;
	idSysMutex                                                      mutex;
	bool                                                            trackLiveness;
};
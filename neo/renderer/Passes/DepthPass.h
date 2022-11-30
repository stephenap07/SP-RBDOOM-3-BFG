#pragma once

#include "GeometryPasses.h"

class MaterialBindingCache;
class idRenderProgManager;

class DepthPass : public idGeometryPass
{
public:
	union PipelineKey
	{
		struct
		{
			nvrhi::RasterCullMode cullMode : 2;
			bool alphaTested : 1;
			bool frontCounterClockwise : 1;
			bool reverseDepth : 1;
		} bits;
		uint32_t value;

		static constexpr size_t Count = 1 << 5;
	};

	class Context : public idGeometryPassContext
	{
	public:
		PipelineKey keyTemplate;

		Context()
		{
			keyTemplate.value = 0;
		}
	};

	struct CreateParameters
	{
		std::shared_ptr<MaterialBindingCache> materialBindings;
		int depthBias = 0;
		float depthBiasClamp = 0.f;
		float slopeScaledDepthBias = 0.f;
		bool trackLiveness = true;
		uint32_t numConstantBufferVersions = 16;
	};

	DepthPass() {}

	virtual void    Init( nvrhi::DeviceHandle deviceHandle, CommonRenderPasses* inCommonPasses, idRenderProgManager& progManager, const CreateParameters& params );
	void            ResetBindingCache() const;

	void            SetupView( idGeometryPassContext& abstractContext, nvrhi::ICommandList* commandList, const viewDef_t* view, const viewDef_t* viewPrev ) override;
	bool            SetupMaterial( idGeometryPassContext& abstractContext, drawSurf_t* drawSurf, nvrhi::RasterCullMode cullMode, nvrhi::GraphicsState& state ) override;
	void            SetupInputBuffers( idGeometryPassContext& abstractContext, drawSurf_t* drawSurf, nvrhi::GraphicsState& state ) override;
	void            SetPushConstants( idGeometryPassContext& abstractContext, nvrhi::ICommandList* commandList, nvrhi::GraphicsState& state, nvrhi::DrawArguments& args ) override;

protected:

	virtual nvrhi::InputLayoutHandle        CreateInputLayout( nvrhi::IShader* vertexShader, const CreateParameters& parms );
	virtual nvrhi::ShaderHandle             CreateVertexShader( idRenderProgManager& renderProgManager, const CreateParameters& params );
	virtual nvrhi::ShaderHandle             CreatePixelShader( idRenderProgManager& renderProgManager, const CreateParameters& params );
	void                                    CreateViewBindings( nvrhi::BindingLayoutHandle& layout, nvrhi::BindingSetHandle& set, const CreateParameters& parms );
	nvrhi::GraphicsPipelineHandle           CreateGraphicsPipeline( PipelineKey key, nvrhi::IFramebuffer* framebuffer );
	std::shared_ptr<MaterialBindingCache>   CreateMaterialBindingCache();

	nvrhi::DeviceHandle                     device;
	nvrhi::InputLayoutHandle                inputLayout;
	nvrhi::ShaderHandle                     vertexShader;
	nvrhi::ShaderHandle                     pixelShader;
	nvrhi::BindingLayoutHandle              viewBindingLayout;
	nvrhi::BufferHandle                     depthCb;
	nvrhi::BindingSetHandle                 viewBindingSet;
	nvrhi::GraphicsPipelineHandle           pipelines[PipelineKey::Count];
	idSysMutex                              mutex;

	int                                     depthBias = 0;
	float                                   depthBiasClamp = 0.f;
	float                                   slopeScaledDepthBias = 0.f;
	bool                                    trackLiveness = true;

	CommonRenderPasses*                     commonPasses;
	std::shared_ptr<MaterialBindingCache>   materialBindings;
};


#ifndef RENDERER_PASSES_FORWARDSHADINGPASS_H_
#define RENDERER_PASSES_FORWARDSHADINGPASS_H_

#include "GeometryPasses.h"

class ForwardShadingPass : IGeometryPass
{
public:

	ForwardShadingPass( ) = default;
	virtual ~ForwardShadingPass( ) = default;

	void Init( nvrhi::DeviceHandle deviceHandle );

	void DrawInteractions( nvrhi::ICommandList* commandList, const viewDef_t* _viewDef );
	void ShadowMapPass( nvrhi::ICommandList* commandList, const drawSurf_t* drawSurfs, const viewLight_t* vLight, int side );

protected:

	nvrhi::DeviceHandle			device;
	nvrhi::BindingLayoutHandle	geometryBindingLayout;
	nvrhi::BindingLayoutHandle  texturedBindingLayout;
	nvrhi::BindingSetDesc		geometryBindingSetDesc;

	nvrhi::GraphicsPipelineHandle CreateGraphicsPipeline( nvrhi::IFramebuffer* framebuffer );

public:
	
	void SetupView(nvrhi::ICommandList* commandList, viewDef_t* viewDef ) override;
	bool SetupMaterial(const idMaterial* material, nvrhi::RasterCullMode cullMode, nvrhi::GraphicsState& state) override;
	void SetupInputBuffers(const drawSurf_t* drawSurf, nvrhi::GraphicsState& state) override;
	void SetPushConstants(nvrhi::ICommandList* commandList, nvrhi::GraphicsState& state, nvrhi::DrawArguments& args) override;

private:

	idRenderMatrix		shadowV[6];				// shadow depth view matrix
	idRenderMatrix		shadowP[6];				// shadow depth projection matrix
};

#endif
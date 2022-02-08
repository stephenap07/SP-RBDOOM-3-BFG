#ifndef RENDERER_PASSES_GBUFFERFILLPASS_H_
#define RENDERER_PASSES_GBUFFERFILLPASS_H_

#include "GeometryPasses.h"

// "Light" G-Buffer that renders the normals of the geometry
class GBufferFillPass : IGeometryPass
{
public:

	GBufferFillPass( ) = default;
	virtual ~GBufferFillPass( ) = default;

	void Init( nvrhi::DeviceHandle deviceHandle );
	void RenderView( nvrhi::ICommandList* commandList, const drawSurf_t* const* drawSurfs, int numDrawSurfs, bool fillGbuffer );

protected:

	nvrhi::DeviceHandle			device;
	nvrhi::BindingLayoutHandle	geometryBindingLayout;
	nvrhi::BindingLayoutHandle  texturedBindingLayout;
	nvrhi::BindingSetDesc		geometryBindingSetDesc;

	nvrhi::GraphicsPipelineHandle CreateGraphicsPipeline( nvrhi::IFramebuffer* framebuffer );

	void DrawElementsWithCounters( const drawSurf_t* surf );

public:
	
	void SetupView(nvrhi::ICommandList* commandList, viewDef_t* viewDef ) override;
	bool SetupMaterial(const idMaterial* material, nvrhi::RasterCullMode cullMode, nvrhi::GraphicsState& state) override;
	void SetupInputBuffers(const drawSurf_t* drawSurf, nvrhi::GraphicsState& state) override;
	void SetPushConstants(nvrhi::ICommandList* commandList, nvrhi::GraphicsState& state, nvrhi::DrawArguments& args) override;

};

#endif
#include "precompiled.h"
#pragma hdrstop

#include "renderer/RenderCommon.h"

#include "DepthPass.h"

#include "CommonPasses.h"

#include <nvrhi/utils.h>

void DepthPass::Init( nvrhi::DeviceHandle deviceHandle, CommonRenderPasses* inCommonPasses, idRenderProgManager& progManager, const CreateParameters& parms )
{
	device = deviceHandle;
	commonPasses = inCommonPasses;

	vertexShader = CreateVertexShader( renderProgManager, parms );
	pixelShader = CreatePixelShader( renderProgManager, parms );
	inputLayout = CreateInputLayout( vertexShader, parms );

	if( parms.materialBindings )
	{
		materialBindings = parms.materialBindings;
	}
	else
	{
		materialBindings = CreateMaterialBindingCache();
	}

	depthCb = device->createBuffer( nvrhi::utils::CreateVolatileConstantBufferDesc(
										sizeof( idRenderMatrix ), "DepthPassConstants", parms.numConstantBufferVersions ) );

	CreateViewBindings( viewBindingLayout, viewBindingSet, parms );

	depthBias = parms.depthBias;
	depthBiasClamp = parms.depthBiasClamp;
	slopeScaledDepthBias = parms.slopeScaledDepthBias;
}

void DepthPass::ResetBindingCache() const
{
	materialBindings->Clear();
}

void DepthPass::SetupView( idGeometryPassContext& abstractContext, nvrhi::ICommandList* commandList, const viewDef_t* view, const viewDef_t* viewPrev )
{
	auto& context = static_cast<Context&>( abstractContext );

	commandList->writeBuffer( depthCb, &view->projectionRenderMatrix, sizeof( idRenderMatrix ) );

	context.keyTemplate.bits.frontCounterClockwise = !view->isMirror;
	context.keyTemplate.bits.reverseDepth = false; // put this in the view?
}

bool DepthPass::SetupMaterial( idGeometryPassContext& abstractContext, drawSurf_t* drawSurf, nvrhi::RasterCullMode cullMode, nvrhi::GraphicsState& state )
{
	auto& context = static_cast<Context&>( abstractContext );

	PipelineKey key = context.keyTemplate;
	key.bits.cullMode = cullMode;

	const idMaterial* shader = drawSurf->material;

	// get the expressions for conditionals / color / texcoords
	const float* regs = drawSurf->shaderRegisters;

	// if all stages of a material have been conditioned off, don't do anything
	int stage = 0;
	for( ; stage < shader->GetNumStages(); stage++ )
	{
		const shaderStage_t* pStage = shader->GetStage( stage );
		// check the stage enable condition
		if( regs[pStage->conditionRegister] != 0 )
		{
			break;
		}
	}

	if( stage == shader->GetNumStages() )
	{
		return false;
	}

	if( shader->Coverage() == MC_PERFORATED )
	{
		nvrhi::IBindingSet* materialBindingSet = materialBindings->GetMaterialBindingSet( drawSurf );

		if( !materialBindingSet )
		{
			return false;
		}

		state.bindings = { viewBindingSet, materialBindingSet };
		key.bits.alphaTested = true;
	}
	else if( shader->Coverage() == MC_OPAQUE )
	{
		state.bindings = { viewBindingSet };
		key.bits.alphaTested = false;
	}
	else
	{
		return false;
	}

	nvrhi::GraphicsPipelineHandle& pipeline = pipelines[key.value];

	if( !pipeline )
	{
		mutex.Lock();

		if( !pipeline )
		{
			pipeline = CreateGraphicsPipeline( key, state.framebuffer );
		}

		mutex.Unlock();

		if( !pipeline )
		{
			return false;
		}
	}

	assert( pipeline->getFramebufferInfo() == state.framebuffer->getFramebufferInfo() );

	state.pipeline = pipeline;
	return true;
}

void DepthPass::SetupInputBuffers( idGeometryPassContext& abstractContext, drawSurf_t* surf, nvrhi::GraphicsState& state )
{
	idVertexBuffer vertexBuffer;
	if( surf->skinnedCache != 0 )
	{
		vertexCache.GetSkinnedVertexBuffer( surf->skinnedCache, &vertexBuffer );
	}
	else
	{
		vertexCache.GetVertexBuffer( surf->ambientCache, &vertexBuffer );
	}

	idIndexBuffer indexBuffer;
	vertexCache.GetIndexBuffer( surf->indexCache, &indexBuffer );

	idVertexBuffer instanceBuffer;
	vertexCache.GetInstanceBuffer( surf->space->instanceCache, &instanceBuffer );

	state.vertexBuffers =
	{
		{ vertexBuffer.GetAPIObject(), 0, 0 },
		{ vertexBuffer.GetAPIObject(), 1, 0 },
		{ instanceBuffer.GetAPIObject(), 2, 0 }
	};

	nvrhi::IBuffer* indexPtr = indexBuffer.GetAPIObject();

	state.indexBuffer = { indexPtr, nvrhi::Format::R16_UINT, 0 };
}

void DepthPass::SetPushConstants( idGeometryPassContext& abstractContext, nvrhi::ICommandList* commandList, nvrhi::GraphicsState& state, nvrhi::DrawArguments& args )
{
}

nvrhi::InputLayoutHandle DepthPass::CreateInputLayout( nvrhi::IShader* vertexShader, const CreateParameters& parms )
{
	nvrhi::VertexAttributeDesc inputDescs[] =
	{
		GetVertexAttributeDesc( VERTEXATTRIBUTE_POSITION, "POSITION", 0 ),
		GetVertexAttributeDesc( VERTEXATTRIBUTE_TEXCOORD1, "TEXCOORD", 1 ),
		GetVertexAttributeDesc( VERTEXATTRIBUTE_TRANSFORM, "TRANSFORM", 2 )
	};

	return device->createInputLayout( inputDescs, 3, vertexShader );
}

nvrhi::ShaderHandle DepthPass::CreateVertexShader( idRenderProgManager& renderProgManager, const CreateParameters& params )
{
	idList<shaderMacro_t> macros;
	int vertexShaderIndex = renderProgManager.FindShader( "builtin/depth2", SHADER_STAGE_VERTEX, "", macros, true, LAYOUT_DRAW_DEPTH );
	return renderProgManager.GetShader( vertexShaderIndex );
}

nvrhi::ShaderHandle DepthPass::CreatePixelShader( idRenderProgManager& renderProgManager, const CreateParameters& params )
{
	idList<shaderMacro_t> macros;
	int pixelShaderIndex = renderProgManager.FindShader( "builtin/depth2", SHADER_STAGE_FRAGMENT, "", macros, true, LAYOUT_DRAW_DEPTH );
	return renderProgManager.GetShader( pixelShaderIndex );
}

void DepthPass::CreateViewBindings( nvrhi::BindingLayoutHandle& layout, nvrhi::BindingSetHandle& set, const CreateParameters& parms )
{
	nvrhi::BindingSetDesc bindingSetDesc;
	bindingSetDesc.bindings =
	{
		nvrhi::BindingSetItem::ConstantBuffer( 0, depthCb )
	};
	bindingSetDesc.trackLiveness = trackLiveness;

	nvrhi::utils::CreateBindingSetAndLayout( device, nvrhi::ShaderType::Vertex, 0, bindingSetDesc, layout, set );
}

nvrhi::GraphicsPipelineHandle DepthPass::CreateGraphicsPipeline( PipelineKey key, nvrhi::IFramebuffer* framebuffer )
{
	nvrhi::GraphicsPipelineDesc pipelineDesc;
	pipelineDesc.inputLayout = inputLayout;
	pipelineDesc.VS = vertexShader;
	pipelineDesc.renderState.rasterState.depthBias = depthBias;
	pipelineDesc.renderState.rasterState.depthBiasClamp = depthBiasClamp;
	pipelineDesc.renderState.rasterState.slopeScaledDepthBias = slopeScaledDepthBias;
	pipelineDesc.renderState.rasterState.frontCounterClockwise = key.bits.frontCounterClockwise;
	pipelineDesc.renderState.rasterState.cullMode = key.bits.cullMode;
	pipelineDesc.renderState.depthStencilState.depthFunc = key.bits.reverseDepth
			? nvrhi::ComparisonFunc::GreaterOrEqual
			: nvrhi::ComparisonFunc::LessOrEqual;

	if( key.bits.alphaTested )
	{
		pipelineDesc.PS = pixelShader;
		pipelineDesc.bindingLayouts = { viewBindingLayout, materialBindings->GetLayout() };
	}
	else
	{
		pipelineDesc.PS = nullptr;
		pipelineDesc.bindingLayouts = { viewBindingLayout };
	}

	return device->createGraphicsPipeline( pipelineDesc, framebuffer );
}

std::shared_ptr<MaterialBindingCache> DepthPass::CreateMaterialBindingCache()
{
	std::vector<MaterialResourceBinding> materialBindings =
	{
		{ MaterialResource::DiffuseTexture, 0 },
		{ MaterialResource::Sampler, 0 },
		{ MaterialResource::ConstantBuffer, 1 }
	};

	return std::make_shared<MaterialBindingCache>(
			   device,
			   nvrhi::ShaderType::Pixel,
			   0, // register space
			   materialBindings,
			   commonPasses->m_AnisotropicWrapSampler,
			   commonPasses->m_GrayTexture,
			   commonPasses->m_BlackTexture );
}

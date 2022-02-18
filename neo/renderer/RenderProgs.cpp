/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2018 Robert Beckebans

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

#include "RenderCommon.h"
#include <sys/DeviceManager.h>
#include <nvrhi/utils.h>

idRenderProgManager renderProgManager;

/*
================================================================================================
idRenderProgManager::idRenderProgManager()
================================================================================================
*/
idRenderProgManager::idRenderProgManager()
{
}

/*
================================================================================================
idRenderProgManager::~idRenderProgManager()
================================================================================================
*/
idRenderProgManager::~idRenderProgManager()
{
}

/*
================================================================================================
R_ReloadShaders
================================================================================================
*/
static void R_ReloadShaders( const idCmdArgs& args )
{
	renderProgManager.KillAllShaders();
	renderProgManager.LoadAllShaders();
}

/*
================================================================================================
idRenderProgManager::Init()
================================================================================================
*/
void idRenderProgManager::Init( nvrhi::IDevice* _device )
{
	common->Printf( "----- Initializing Render Shaders -----\n" );

	device = _device;

	uniforms.SetNum( RENDERPARM_TOTAL, vec4_zero );

	constantBuffer = device->createBuffer(
		nvrhi::utils::CreateVolatileConstantBufferDesc( uniforms.Allocated(),
			"RenderParams",
			c_MaxRenderPassConstantBufferVersions ) );

	for( int i = 0; i < MAX_BUILTINS; i++ )
	{
		builtinShaders[i] = -1;
	}

	// === Main draw vertex layout ===
	vertexLayoutDescs.SetNum( NUM_VERTEX_LAYOUTS, idList<nvrhi::VertexAttributeDesc>( ) );

	vertexLayoutDescs[LAYOUT_DRAW_VERT].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "POSITION" )
		.setFormat( nvrhi::Format::RGB32_FLOAT )
		.setOffset( offsetof( idDrawVert, xyz ) )
		.setElementStride( sizeof( idDrawVert ) ) );

	vertexLayoutDescs[LAYOUT_DRAW_VERT].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "TEXCOORD" )
		.setFormat( nvrhi::Format::RG16_FLOAT )
		.setOffset( offsetof( idDrawVert, st ) )
		.setElementStride( sizeof( idDrawVert ) ) );
	
	vertexLayoutDescs[LAYOUT_DRAW_VERT].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "NORMAL" )
		.setFormat( nvrhi::Format::RGBA8_UNORM )
		.setOffset( offsetof( idDrawVert, normal ) )
		.setElementStride( sizeof( idDrawVert ) ) );

	vertexLayoutDescs[LAYOUT_DRAW_VERT].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "TANGENT" )
		.setFormat( nvrhi::Format::RGBA8_UNORM )
		.setOffset( offsetof( idDrawVert, tangent ) )
		.setElementStride( sizeof( idDrawVert ) ) );

	vertexLayoutDescs[LAYOUT_DRAW_VERT].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "COLOR" )
		.setArraySize( 2 )
		.setFormat( nvrhi::Format::RGBA8_UNORM )
		.setOffset( offsetof( idDrawVert, color ) )
		.setElementStride( sizeof( idDrawVert ) ) );

	// === Shadow vertex ===

	vertexLayoutDescs[LAYOUT_DRAW_SHADOW_VERT].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "POSITION" )
		.setFormat( nvrhi::Format::RGBA32_FLOAT )
		.setOffset( offsetof( idShadowVert, xyzw ) )
		.setElementStride( sizeof( idShadowVert ) ) );

	// === Shadow vertex skinned ===

	vertexLayoutDescs[LAYOUT_DRAW_SHADOW_VERT_SKINNED].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "POSITION" )
		.setFormat( nvrhi::Format::RGBA32_FLOAT )
		.setOffset( offsetof( idShadowVertSkinned, xyzw ) )
		.setElementStride( sizeof( idShadowVertSkinned ) ) );

	vertexLayoutDescs[LAYOUT_DRAW_SHADOW_VERT_SKINNED].Append(
		nvrhi::VertexAttributeDesc( )
		.setName( "COLOR" )
		.setArraySize( 2 )
		.setFormat( nvrhi::Format::RGBA8_UNORM )
		.setOffset( offsetof( idShadowVertSkinned, color ) )
		.setElementStride( sizeof( idShadowVertSkinned ) ) );

	bindingLayouts.SetNum( NUM_BINDING_LAYOUTS );

	auto defaultLayoutDesc = nvrhi::BindingLayoutDesc( )
		.setVisibility( nvrhi::ShaderType::All )
		.addItem( nvrhi::BindingLayoutItem::VolatileConstantBuffer( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Sampler( 0 ) );

	bindingLayouts[BINDING_LAYOUT_DEFAULT] = device->createBindingLayout( defaultLayoutDesc );

	auto lightGridLayoutDesc = nvrhi::BindingLayoutDesc( )
		.setVisibility( nvrhi::ShaderType::All )
		.addItem( nvrhi::BindingLayoutItem::VolatileConstantBuffer( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 0 ) ) // normal
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 1 ) ) // specular
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 2 ) ) // base color
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 3 ) ) // brdf lut
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 4 ) ) // ssao
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 7 ) ) // irradiance cube map
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 8 ) ) // radiance cube map 1
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 9 ) ) // radiance cube map 2
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 10 ) ) // radiance cube map 3
		.addItem( nvrhi::BindingLayoutItem::Sampler( 0 ) ) // normal sampler
		.addItem( nvrhi::BindingLayoutItem::Sampler( 1 ) ) // specular sampler
		.addItem( nvrhi::BindingLayoutItem::Sampler( 2 ) ) // base color sampler
		.addItem( nvrhi::BindingLayoutItem::Sampler( 3 ) ) // brdf lut sampler
		.addItem( nvrhi::BindingLayoutItem::Sampler( 4 ) ) // ssao sampler
		.addItem( nvrhi::BindingLayoutItem::Sampler( 7 ) ) // irradiance sampler
		.addItem( nvrhi::BindingLayoutItem::Sampler( 8 ) ) // radiance sampler 1
		.addItem( nvrhi::BindingLayoutItem::Sampler( 9 ) ) // radiance sampler 2
		.addItem( nvrhi::BindingLayoutItem::Sampler( 10 ) ); // radiance sampler 3

	bindingLayouts[BINDING_LAYOUT_LIGHTGRID] = device->createBindingLayout( lightGridLayoutDesc );

	auto blitLayoutDesc = nvrhi::BindingLayoutDesc( )
		.setVisibility( nvrhi::ShaderType::All )
		.addItem( nvrhi::BindingLayoutItem::VolatileConstantBuffer( 0 ) ); // blit constants

	bindingLayouts[BINDING_LAYOUT_BLIT] = device->createBindingLayout( blitLayoutDesc );

	auto aoLayoutDesc = nvrhi::BindingLayoutDesc( )
		.setVisibility( nvrhi::ShaderType::All )
		.addItem( nvrhi::BindingLayoutItem::VolatileConstantBuffer( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 1 ) )
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 2 ) )
		.addItem( nvrhi::BindingLayoutItem::Sampler( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Sampler( 1 ) )
		.addItem( nvrhi::BindingLayoutItem::Sampler( 2 ) );

	bindingLayouts[BINDING_LAYOUT_DRAW_AO] = device->createBindingLayout( aoLayoutDesc );

	auto aoLayoutDesc2 = nvrhi::BindingLayoutDesc( )
		.setVisibility( nvrhi::ShaderType::All )
		.addItem( nvrhi::BindingLayoutItem::VolatileConstantBuffer( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Texture_SRV( 0 ) )
		.addItem( nvrhi::BindingLayoutItem::Sampler( 0 ) );

	bindingLayouts[BINDING_LAYOUT_DRAW_AO1] = device->createBindingLayout( aoLayoutDesc2 );

	// RB: added checks for GPU skinning
	struct builtinShaders_t
	{
		int					index;
		const char*			name;
		const char*			nameOutSuffix;
		uint32				shaderFeatures;
		bool				requireGPUSkinningSupport;
		rpStage_t			stages;
		vertexLayoutType_t	layout;
		bindingLayoutType_t bindingLayout;
	} builtins[] =
	{
		{ BUILTIN_GUI, "builtin/gui", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_COLOR, "builtin/color", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		// RB begin
		{ BUILTIN_COLOR_SKINNED, "builtin/color", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_VERTEX_COLOR, "builtin/vertex_color", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_AMBIENT_LIGHTING, "builtin/lighting/ambient_lighting", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_AMBIENT_LIGHTING_SKINNED, "builtin/lighting/ambient_lighting", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_AMBIENT_LIGHTING_IBL, "builtin/lighting/ambient_lighting_IBL", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_AMBIENT_LIGHTING_IBL_SKINNED, "builtin/lighting/ambient_lighting_IBL", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_AMBIENT_LIGHTING_IBL_PBR, "builtin/lighting/ambient_lighting_IBL", "_PBR", BIT( USE_PBR ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_AMBIENT_LIGHTING_IBL_PBR_SKINNED, "builtin/lighting/ambient_lighting_IBL", "_PBR_skinned", BIT( USE_GPU_SKINNING | USE_PBR ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_AMBIENT_LIGHTGRID_IBL, "builtin/lighting/ambient_lightgrid_IBL", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_LIGHTGRID },
		{ BUILTIN_AMBIENT_LIGHTGRID_IBL_SKINNED, "builtin/lighting/ambient_lightgrid_IBL", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_LIGHTGRID },
		{ BUILTIN_AMBIENT_LIGHTGRID_IBL_PBR, "builtin/lighting/ambient_lightgrid_IBL", "_PBR", BIT( USE_PBR ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_LIGHTGRID },
		{ BUILTIN_AMBIENT_LIGHTGRID_IBL_PBR_SKINNED, "builtin/lighting/ambient_lightgrid_IBL", "_PBR_skinned", BIT( USE_GPU_SKINNING | USE_PBR ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_LIGHTGRID },

		{ BUILTIN_SMALL_GEOMETRY_BUFFER, "builtin/gbuffer", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_SMALL_GEOMETRY_BUFFER_SKINNED, "builtin/gbuffer", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		// RB end
		{ BUILTIN_TEXTURED, "builtin/texture", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_TEXTURE_VERTEXCOLOR, "builtin/texture_color", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_TEXTURE_VERTEXCOLOR_SRGB, "builtin/texture_color", "_sRGB", BIT( USE_SRGB ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_TEXTURE_VERTEXCOLOR_SKINNED, "builtin/texture_color", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_TEXTURE_TEXGEN_VERTEXCOLOR, "builtin/texture_color_texgen", "",  0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		// RB begin
		{ BUILTIN_INTERACTION, "builtin/lighting/interaction", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_INTERACTION_SKINNED, "builtin/lighting/interaction", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_INTERACTION_AMBIENT, "builtin/lighting/interactionAmbient", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_INTERACTION_AMBIENT_SKINNED, "builtin/lighting/interactionAmbient_skinned", "", 0, true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_INTERACTION_SHADOW_MAPPING_SPOT, "builtin/lighting/interactionSM", "_spot", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_INTERACTION_SHADOW_MAPPING_SPOT_SKINNED, "builtin/lighting/interactionSM", "_spot_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_INTERACTION_SHADOW_MAPPING_POINT, "builtin/lighting/interactionSM", "_point", BIT( LIGHT_POINT ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_INTERACTION_SHADOW_MAPPING_POINT_SKINNED, "builtin/lighting/interactionSM", "_point_skinned", BIT( USE_GPU_SKINNING ) | BIT( LIGHT_POINT ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_INTERACTION_SHADOW_MAPPING_PARALLEL, "builtin/lighting/interactionSM", "_parallel", BIT( LIGHT_PARALLEL ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_INTERACTION_SHADOW_MAPPING_PARALLEL_SKINNED, "builtin/lighting/interactionSM", "_parallel_skinned", BIT( USE_GPU_SKINNING ) | BIT( LIGHT_PARALLEL ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		// PBR variants
		{ BUILTIN_PBR_INTERACTION, "builtin/lighting/interaction", "_PBR", BIT( USE_PBR ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_PBR_INTERACTION_SKINNED, "builtin/lighting/interaction", "_skinned_PBR", BIT( USE_GPU_SKINNING ) | BIT( USE_PBR ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_PBR_INTERACTION_AMBIENT, "builtin/lighting/interactionAmbient", "_PBR", BIT( USE_PBR ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_PBR_INTERACTION_AMBIENT_SKINNED, "builtin/lighting/interactionAmbient_skinned", "_PBR", BIT( USE_PBR ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_SPOT, "builtin/lighting/interactionSM", "_spot_PBR", BIT( USE_PBR ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_SPOT_SKINNED, "builtin/lighting/interactionSM", "_spot_skinned_PBR", BIT( USE_GPU_SKINNING ) | BIT( USE_PBR ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_POINT, "builtin/lighting/interactionSM", "_point_PBR", BIT( LIGHT_POINT ) | BIT( USE_PBR ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_POINT_SKINNED, "builtin/lighting/interactionSM", "_point_skinned_PBR", BIT( USE_GPU_SKINNING ) | BIT( LIGHT_POINT ) | BIT( USE_PBR ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_PARALLEL, "builtin/lighting/interactionSM", "_parallel_PBR", BIT( LIGHT_PARALLEL ) | BIT( USE_PBR ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_PARALLEL_SKINNED, "builtin/lighting/interactionSM", "_parallel_skinned_PBR", BIT( USE_GPU_SKINNING ) | BIT( LIGHT_PARALLEL ) | BIT( USE_PBR ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		// debug stuff
		{ BUILTIN_DEBUG_LIGHTGRID, "builtin/debug/lightgrid", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_DEBUG_LIGHTGRID_SKINNED, "builtin/debug/lightgrid", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_DEBUG_OCTAHEDRON, "builtin/debug/octahedron", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_DEBUG_OCTAHEDRON_SKINNED, "builtin/debug/octahedron", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		// RB end

		{ BUILTIN_ENVIRONMENT, "builtin/legacy/environment", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_ENVIRONMENT_SKINNED, "builtin/legacy/environment", "_skinned",  BIT( USE_GPU_SKINNING ), true , SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_BUMPY_ENVIRONMENT, "builtin/legacy/bumpyenvironment", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_BUMPY_ENVIRONMENT_SKINNED, "builtin/legacy/bumpyenvironment", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_DEPTH, "builtin/depth", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_DEPTH_SKINNED, "builtin/depth", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_SHADOW, "builtin/lighting/shadow", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_SHADOW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_SHADOW_SKINNED, "builtin/lighting/shadow", "_skinned", 0, BIT( USE_GPU_SKINNING ), SHADER_STAGE_DEFAULT, LAYOUT_DRAW_SHADOW_VERT_SKINNED, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_SHADOW_DEBUG, "builtin/debug/shadowDebug", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_SHADOW_DEBUG_SKINNED, "builtin/debug/shadowDebug", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_BLENDLIGHT, "builtin/fog/blendlight", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_FOG, "builtin/fog/fog", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_FOG_SKINNED, "builtin/fog/fog", "_skinned", BIT( USE_GPU_SKINNING ), true, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_SKYBOX, "builtin/legacy/skybox", "",  0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_WOBBLESKY, "builtin/legacy/wobblesky", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_POSTPROCESS, "builtin/post/postprocess", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		// RB begin
		{ BUILTIN_SCREEN, "builtin/post/screen", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_TONEMAP, "builtin/post/tonemap", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_BRIGHTPASS, "builtin/post/tonemap", "_brightpass", BIT( BRIGHTPASS ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_HDR_GLARE_CHROMATIC, "builtin/post/hdr_glare_chromatic", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_HDR_DEBUG, "builtin/post/tonemap", "_debug", BIT( HDR_DEBUG ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_SMAA_EDGE_DETECTION, "builtin/post/SMAA_edge_detection", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_SMAA_BLENDING_WEIGHT_CALCULATION, "builtin/post/SMAA_blending_weight_calc", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_SMAA_NEIGHBORHOOD_BLENDING, "builtin/post/SMAA_final", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		{ BUILTIN_AMBIENT_OCCLUSION, "builtin/SSAO/AmbientOcclusion_AO", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DRAW_AO },
		{ BUILTIN_AMBIENT_OCCLUSION_AND_OUTPUT, "builtin/SSAO/AmbientOcclusion_AO", "_write", BIT( BRIGHTPASS ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DRAW_AO },
		{ BUILTIN_AMBIENT_OCCLUSION_BLUR, "builtin/SSAO/AmbientOcclusion_blur", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DRAW_AO },
		{ BUILTIN_AMBIENT_OCCLUSION_BLUR_AND_OUTPUT, "builtin/SSAO/AmbientOcclusion_blur", "_write", BIT( BRIGHTPASS ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DRAW_AO },
		{ BUILTIN_AMBIENT_OCCLUSION_MINIFY, "builtin/SSAO/AmbientOcclusion_minify", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DRAW_AO1 },
		{ BUILTIN_AMBIENT_OCCLUSION_RECONSTRUCT_CSZ, "builtin/SSAO/AmbientOcclusion_minify", "_mip0", BIT( BRIGHTPASS ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DRAW_AO1 },
		{ BUILTIN_DEEP_GBUFFER_RADIOSITY_SSGI, "builtin/SSGI/DeepGBufferRadiosity_radiosity", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_DEEP_GBUFFER_RADIOSITY_BLUR, "builtin/SSGI/DeepGBufferRadiosity_blur", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_DEEP_GBUFFER_RADIOSITY_BLUR_AND_OUTPUT, "builtin/SSGI/DeepGBufferRadiosity_blur", "_write", BIT( BRIGHTPASS ), false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		// RB end
		{ BUILTIN_STEREO_DEGHOST, "builtin/VR/stereoDeGhost", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_STEREO_WARP, "builtin/VR/stereoWarp", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_BINK, "builtin/video/bink", "",  0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_BINK_GUI, "builtin/video/bink_gui", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_STEREO_INTERLACE, "builtin/VR/stereoInterlace", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		{ BUILTIN_MOTION_BLUR, "builtin/post/motionBlur", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },

		// RB begin
		{ BUILTIN_DEBUG_SHADOWMAP, "builtin/debug/debug_shadowmap", "", 0, false, SHADER_STAGE_DEFAULT, LAYOUT_DRAW_VERT, BINDING_LAYOUT_DEFAULT },
		// RB end
	};
	int numBuiltins = sizeof( builtins ) / sizeof( builtins[0] );

	struct newBuiltins_t {
		int						index;
		const char*				name;
		const char*				nameOutSuffix;
		idList<shaderMacro_t>	macros;
		bool					requireGPUSkinningSupport;
		rpStage_t				stages;
		vertexLayoutType_t		layout;
		bindingLayoutType_t		bindingLayout;
	} newBuiltins[] = {
		// SP begin
		{ BUILTIN_BLIT, "builtin/blit", "", { { "TEXTURE_ARRAY", "0" } }, false, SHADER_STAGE_FRAGMENT, LAYOUT_UNKNOWN, BINDING_LAYOUT_BLIT },
		{ BUILTIN_RECT, "builtin/rect", "", { }, false, SHADER_STAGE_VERTEX, LAYOUT_DRAW_VERT, BINDING_LAYOUT_BLIT },
		// SP end
	};

	renderProgs.SetNum( numBuiltins + std::size( newBuiltins ) );

	for( int i = 0; i < numBuiltins; i++ )
	{
		renderProg_t& prog = renderProgs[ i ];

		prog.name = builtins[i].name;
		prog.builtin = true;
		prog.vertexLayout = builtins[i].layout;
		prog.bindingLayoutType = builtins[i].bindingLayout;

		builtinShaders[builtins[i].index] = i;

		if( builtins[i].requireGPUSkinningSupport && !glConfig.gpuSkinningAvailable )
		{
			// RB: don't try to load shaders that would break the GLSL compiler in the OpenGL driver
			continue;
		}

		uint32 shaderFeatures = builtins[i].shaderFeatures;
		if( builtins[i].requireGPUSkinningSupport )
		{
			shaderFeatures |= BIT( USE_GPU_SKINNING );
		}

		int vIndex = -1;
		if( builtins[ i ].stages & SHADER_STAGE_VERTEX )
		{
			vIndex = FindShader( builtins[ i ].name, SHADER_STAGE_VERTEX, builtins[i].nameOutSuffix, shaderFeatures, true, builtins[i].layout );
		}

		int fIndex = -1;
		if( builtins[ i ].stages & SHADER_STAGE_FRAGMENT )
		{
			fIndex = FindShader( builtins[ i ].name, SHADER_STAGE_FRAGMENT, builtins[i].nameOutSuffix, shaderFeatures, true, builtins[i].layout );
		}

		idLib::Printf( "Loading shader program %s\n", prog.name.c_str() );

		LoadProgram( i, vIndex, fIndex );
	}

	for( int i = 0; i < std::size( newBuiltins ); i++ )
	{
		renderProg_t& prog = renderProgs[i + std::size(newBuiltins)];

		prog.name = newBuiltins[i].name;
		prog.builtin = true;
		prog.vertexLayout = newBuiltins[i].layout;
		prog.bindingLayoutType = newBuiltins[i].bindingLayout;

		builtinShaders[newBuiltins[i].index] = i + numBuiltins;

		if( newBuiltins[i].requireGPUSkinningSupport && !glConfig.gpuSkinningAvailable )
		{
			// RB: don't try to load shaders that would break the GLSL compiler in the OpenGL driver
			continue;
		}

		int vIndex = -1;
		if( newBuiltins[i].stages & SHADER_STAGE_VERTEX )
		{
			vIndex = FindShader( newBuiltins[i].name, SHADER_STAGE_VERTEX, newBuiltins[i].nameOutSuffix, newBuiltins[i].macros, true, newBuiltins[i].layout );
		}

		int fIndex = -1;
		if( newBuiltins[i].stages & SHADER_STAGE_FRAGMENT )
		{
			fIndex = FindShader( newBuiltins[i].name, SHADER_STAGE_FRAGMENT, newBuiltins[i].nameOutSuffix, newBuiltins[i].macros, true, newBuiltins[i].layout );
		}

		idLib::Printf( "Loading shader program %s\n", prog.name.c_str( ) );

		LoadProgram( i + numBuiltins, vIndex, fIndex );
	}

	r_useHalfLambertLighting.ClearModified();
	r_useHDR.ClearModified();
	r_usePBR.ClearModified();
	r_pbrDebug.ClearModified();

	uniforms.SetNum( RENDERPARM_TOTAL, vec4_zero );

	if( glConfig.gpuSkinningAvailable )
	{
		renderProgs[builtinShaders[BUILTIN_TEXTURE_VERTEXCOLOR_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_INTERACTION_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_INTERACTION_AMBIENT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_ENVIRONMENT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_BUMPY_ENVIRONMENT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_DEPTH_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_SHADOW_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_SHADOW_DEBUG_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_FOG_SKINNED]].usesJoints = true;
		// RB begin
		renderProgs[builtinShaders[BUILTIN_DEBUG_LIGHTGRID_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_DEBUG_OCTAHEDRON_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_AMBIENT_LIGHTING_SKINNED]].usesJoints = true;

		renderProgs[builtinShaders[BUILTIN_AMBIENT_LIGHTING_IBL_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_AMBIENT_LIGHTING_IBL_PBR_SKINNED]].usesJoints = true;

		renderProgs[builtinShaders[BUILTIN_AMBIENT_LIGHTGRID_IBL_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_AMBIENT_LIGHTGRID_IBL_PBR_SKINNED]].usesJoints = true;

		renderProgs[builtinShaders[BUILTIN_SMALL_GEOMETRY_BUFFER_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_INTERACTION_SHADOW_MAPPING_SPOT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_INTERACTION_SHADOW_MAPPING_POINT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_INTERACTION_SHADOW_MAPPING_PARALLEL_SKINNED]].usesJoints = true;

		// PBR variants
		renderProgs[builtinShaders[BUILTIN_PBR_INTERACTION_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_PBR_INTERACTION_AMBIENT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_SPOT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_POINT_SKINNED]].usesJoints = true;
		renderProgs[builtinShaders[BUILTIN_PBR_INTERACTION_SHADOW_MAPPING_PARALLEL_SKINNED]].usesJoints = true;
		// RB end
	}

	cmdSystem->AddCommand( "reloadShaders", R_ReloadShaders, CMD_FL_RENDERER, "reloads shaders" );
}

/*
================================================================================================
idRenderProgManager::LoadAllShaders()
================================================================================================
*/
void idRenderProgManager::LoadAllShaders()
{
	for( int i = 0; i < shaders.Num(); i++ )
	{
		LoadShader( i, shaders[i].stage );
	}

	for( int i = 0; i < renderProgs.Num(); ++i )
	{
		if( renderProgs[i].vertexShaderIndex == -1 || renderProgs[i].fragmentShaderIndex == -1 )
		{
			// RB: skip reloading because we didn't load it initially
			continue;
		}

		LoadProgram( i, renderProgs[i].vertexShaderIndex, renderProgs[i].fragmentShaderIndex );
	}
}



/*
================================================================================================
idRenderProgManager::Shutdown()
================================================================================================
*/
void idRenderProgManager::Shutdown()
{
	KillAllShaders();
}

/*
================================================================================================
idRenderProgManager::FindVertexShader
================================================================================================
*/
int idRenderProgManager::FindShader( const char* name, rpStage_t stage )
{
	idStr shaderName( name );
	shaderName.StripFileExtension();

	for( int i = 0; i < shaders.Num(); i++ )
	{
		shader_t& shader = shaders[ i ];
		if( shader.name.Icmp( shaderName.c_str() ) == 0 && shader.stage == stage )
		{
			LoadShader( i, stage );
			return i;
		}
	}

	// Load it.
	shader_t shader;
	shader.name = shaderName;
	shader.stage = stage;

	for( int i = 0; i < MAX_SHADER_MACRO_NAMES; i++ )
	{
		int feature = ( bool )( 0 & BIT( i ) );
		idStr macroName( GetGLSLMacroName( ( shaderFeature_t )i ) );
		idStr value( feature );
		shader.macros.Append( shaderMacro_t( macroName, value ) );
	}

	int index = shaders.Append( shader );
	LoadShader( index, stage );

	return index;
}

int idRenderProgManager::FindShader( const char* name, rpStage_t stage, const char* nameOutSuffix, uint32 features, bool builtin, vertexLayoutType_t vertexLayout )
{
	idStr shaderName( name );
	shaderName.StripFileExtension( );

	for( int i = 0; i < shaders.Num( ); i++ )
	{
		shader_t& shader = shaders[i];
		if( shader.name.Icmp( shaderName ) == 0 && shader.stage == stage && shader.nameOutSuffix.Icmp( nameOutSuffix ) == 0 )
		{
			LoadShader( i, stage );
			return i;
		}
	}

	// Load it.
	shader_t shader;
	shader.name = shaderName;
	shader.nameOutSuffix = nameOutSuffix;
	shader.shaderFeatures = features;
	shader.builtin = builtin;
	shader.stage = stage;

	for( int i = 0; i < MAX_SHADER_MACRO_NAMES; i++ )
	{
		int feature = (bool)( features & BIT( i ) );
		idStr macroName( GetGLSLMacroName( ( shaderFeature_t )i ) );
		idStr value( feature );
		shader.macros.Append( shaderMacro_t( macroName, value ) );
	}

	int index = shaders.Append( shader );
	LoadShader( index, stage );

	return index;
}

int idRenderProgManager::FindShader( const char* name, rpStage_t stage, const char* nameOutSuffix, const idList<shaderMacro_t>& macros, bool builtin, vertexLayoutType_t vertexLayout )
{
	idStr shaderName( name );
	shaderName.StripFileExtension( );

	for( int i = 0; i < shaders.Num( ); i++ )
	{
		shader_t& shader = shaders[i];
		if( shader.name.Icmp( shaderName ) == 0 && shader.stage == stage && shader.nameOutSuffix.Icmp( nameOutSuffix ) == 0 )
		{
			LoadShader( i, stage );
			return i;
		}
	}

	// Load it.
	shader_t shader;
	shader.name = shaderName;
	shader.nameOutSuffix = nameOutSuffix;
	shader.shaderFeatures = 0;
	shader.builtin = builtin;
	shader.stage = stage;
	shader.macros = macros;

	int index = shaders.Append( shader );
	LoadShader( index, stage );

	return index;
}

nvrhi::ShaderHandle idRenderProgManager::GetShader( int index )
{
	return shaders[ index ].handle;
}

programInfo_t idRenderProgManager::GetProgramInfo( int index )
{
	programInfo_t info;

	renderProg_t& prog = renderProgs[index];
	if( prog.vertexShaderIndex > -1 && prog.vertexShaderIndex < shaders.Num( ) )
	{
		info.vs = GetShader( prog.vertexShaderIndex );
	}
	if( prog.fragmentShaderIndex > -1 && prog.fragmentShaderIndex < shaders.Num( ) )
	{
		info.ps = GetShader( prog.fragmentShaderIndex );
	}
	info.inputLayout = prog.inputLayout;
	info.bindingLayout = prog.bindingLayout;

	return info;
}

// RB begin
bool idRenderProgManager::IsShaderBound() const
{
	return ( currentIndex != -1 );
}
// RB end

/*
================================================================================================
idRenderProgManager::SetRenderParms
================================================================================================
*/
void idRenderProgManager::SetRenderParms( renderParm_t rp, const float* value, int num )
{
	for( int i = 0; i < num; i++ )
	{
		SetRenderParm( ( renderParm_t )( rp + i ), value + ( i * 4 ) );
	}
}

/*
================================================================================================
idRenderProgManager::SetRenderParm
================================================================================================
*/
void idRenderProgManager::SetRenderParm( renderParm_t rp, const float* value )
{
	SetUniformValue( rp, value );
}


/*
========================
RpPrintState
========================
*/
void RpPrintState( uint64 stateBits )
{

	// culling
	idLib::Printf( "Culling: " );
	switch( stateBits & GLS_CULL_BITS )
	{
		case GLS_CULL_FRONTSIDED:
			idLib::Printf( "FRONTSIDED -> BACK" );
			break;
		case GLS_CULL_BACKSIDED:
			idLib::Printf( "BACKSIDED -> FRONT" );
			break;
		case GLS_CULL_TWOSIDED:
			idLib::Printf( "TWOSIDED" );
			break;
		default:
			idLib::Printf( "NA" );
			break;
	}
	idLib::Printf( "\n" );

	// polygon mode
	idLib::Printf( "PolygonMode: %s\n", ( stateBits & GLS_POLYMODE_LINE ) ? "LINE" : "FILL" );

	// color mask
	idLib::Printf( "ColorMask: " );
	idLib::Printf( ( stateBits & GLS_REDMASK ) ? "_" : "R" );
	idLib::Printf( ( stateBits & GLS_GREENMASK ) ? "_" : "G" );
	idLib::Printf( ( stateBits & GLS_BLUEMASK ) ? "_" : "B" );
	idLib::Printf( ( stateBits & GLS_ALPHAMASK ) ? "_" : "A" );
	idLib::Printf( "\n" );

	// blend
	idLib::Printf( "Blend: src=" );
	switch( stateBits & GLS_SRCBLEND_BITS )
	{
		case GLS_SRCBLEND_ZERO:
			idLib::Printf( "ZERO" );
			break;
		case GLS_SRCBLEND_ONE:
			idLib::Printf( "ONE" );
			break;
		case GLS_SRCBLEND_DST_COLOR:
			idLib::Printf( "DST_COLOR" );
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
			idLib::Printf( "ONE_MINUS_DST_COLOR" );
			break;
		case GLS_SRCBLEND_SRC_ALPHA:
			idLib::Printf( "SRC_ALPHA" );
			break;
		case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
			idLib::Printf( "ONE_MINUS_SRC_ALPHA" );
			break;
		case GLS_SRCBLEND_DST_ALPHA:
			idLib::Printf( "DST_ALPHA" );
			break;
		case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
			idLib::Printf( "ONE_MINUS_DST_ALPHA" );
			break;
		default:
			idLib::Printf( "NA" );
			break;
	}
	idLib::Printf( ", dst=" );
	switch( stateBits & GLS_DSTBLEND_BITS )
	{
		case GLS_DSTBLEND_ZERO:
			idLib::Printf( "ZERO" );
			break;
		case GLS_DSTBLEND_ONE:
			idLib::Printf( "ONE" );
			break;
		case GLS_DSTBLEND_SRC_COLOR:
			idLib::Printf( "SRC_COLOR" );
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
			idLib::Printf( "ONE_MINUS_SRC_COLOR" );
			break;
		case GLS_DSTBLEND_SRC_ALPHA:
			idLib::Printf( "SRC_ALPHA" );
			break;
		case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
			idLib::Printf( "ONE_MINUS_SRC_ALPHA" );
			break;
		case GLS_DSTBLEND_DST_ALPHA:
			idLib::Printf( "DST_ALPHA" );
			break;
		case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
			idLib::Printf( "ONE_MINUS_DST_ALPHA" );
			break;
		default:
			idLib::Printf( "NA" );
	}
	idLib::Printf( "\n" );

	// depth func
	idLib::Printf( "DepthFunc: " );
	switch( stateBits & GLS_DEPTHFUNC_BITS )
	{
		case GLS_DEPTHFUNC_EQUAL:
			idLib::Printf( "EQUAL" );
			break;
		case GLS_DEPTHFUNC_ALWAYS:
			idLib::Printf( "ALWAYS" );
			break;
		case GLS_DEPTHFUNC_LESS:
			idLib::Printf( "LEQUAL" );
			break;
		case GLS_DEPTHFUNC_GREATER:
			idLib::Printf( "GEQUAL" );
			break;
		default:
			idLib::Printf( "NA" );
			break;
	}
	idLib::Printf( "\n" );

	// depth mask
	idLib::Printf( "DepthWrite: %s\n", ( stateBits & GLS_DEPTHMASK ) ? "FALSE" : "TRUE" );

	// depth bounds
	idLib::Printf( "DepthBounds: %s\n", ( stateBits & GLS_DEPTH_TEST_MASK ) ? "TRUE" : "FALSE" );

	// depth bias
	idLib::Printf( "DepthBias: %s\n", ( stateBits & GLS_POLYGON_OFFSET ) ? "TRUE" : "FALSE" );

	// stencil
	auto printStencil = [&]( stencilFace_t face, uint64 bits, uint64 mask, uint64 ref )
	{
		idLib::Printf( "Stencil: %s, ", ( bits & ( GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS ) ) ? "ON" : "OFF" );
		idLib::Printf( "Face=" );
		switch( face )
		{
			case STENCIL_FACE_FRONT:
				idLib::Printf( "FRONT" );
				break;
			case STENCIL_FACE_BACK:
				idLib::Printf( "BACK" );
				break;
			default:
				idLib::Printf( "BOTH" );
				break;
		}
		idLib::Printf( ", Func=" );
		switch( bits & GLS_STENCIL_FUNC_BITS )
		{
			case GLS_STENCIL_FUNC_NEVER:
				idLib::Printf( "NEVER" );
				break;
			case GLS_STENCIL_FUNC_LESS:
				idLib::Printf( "LESS" );
				break;
			case GLS_STENCIL_FUNC_EQUAL:
				idLib::Printf( "EQUAL" );
				break;
			case GLS_STENCIL_FUNC_LEQUAL:
				idLib::Printf( "LEQUAL" );
				break;
			case GLS_STENCIL_FUNC_GREATER:
				idLib::Printf( "GREATER" );
				break;
			case GLS_STENCIL_FUNC_NOTEQUAL:
				idLib::Printf( "NOTEQUAL" );
				break;
			case GLS_STENCIL_FUNC_GEQUAL:
				idLib::Printf( "GEQUAL" );
				break;
			case GLS_STENCIL_FUNC_ALWAYS:
				idLib::Printf( "ALWAYS" );
				break;
			default:
				idLib::Printf( "NA" );
				break;
		}
		idLib::Printf( ", OpFail=" );
		switch( bits & GLS_STENCIL_OP_FAIL_BITS )
		{
			case GLS_STENCIL_OP_FAIL_KEEP:
				idLib::Printf( "KEEP" );
				break;
			case GLS_STENCIL_OP_FAIL_ZERO:
				idLib::Printf( "ZERO" );
				break;
			case GLS_STENCIL_OP_FAIL_REPLACE:
				idLib::Printf( "REPLACE" );
				break;
			case GLS_STENCIL_OP_FAIL_INCR:
				idLib::Printf( "INCR" );
				break;
			case GLS_STENCIL_OP_FAIL_DECR:
				idLib::Printf( "DECR" );
				break;
			case GLS_STENCIL_OP_FAIL_INVERT:
				idLib::Printf( "INVERT" );
				break;
			case GLS_STENCIL_OP_FAIL_INCR_WRAP:
				idLib::Printf( "INCR_WRAP" );
				break;
			case GLS_STENCIL_OP_FAIL_DECR_WRAP:
				idLib::Printf( "DECR_WRAP" );
				break;
			default:
				idLib::Printf( "NA" );
				break;
		}
		idLib::Printf( ", ZFail=" );
		switch( bits & GLS_STENCIL_OP_ZFAIL_BITS )
		{
			case GLS_STENCIL_OP_ZFAIL_KEEP:
				idLib::Printf( "KEEP" );
				break;
			case GLS_STENCIL_OP_ZFAIL_ZERO:
				idLib::Printf( "ZERO" );
				break;
			case GLS_STENCIL_OP_ZFAIL_REPLACE:
				idLib::Printf( "REPLACE" );
				break;
			case GLS_STENCIL_OP_ZFAIL_INCR:
				idLib::Printf( "INCR" );
				break;
			case GLS_STENCIL_OP_ZFAIL_DECR:
				idLib::Printf( "DECR" );
				break;
			case GLS_STENCIL_OP_ZFAIL_INVERT:
				idLib::Printf( "INVERT" );
				break;
			case GLS_STENCIL_OP_ZFAIL_INCR_WRAP:
				idLib::Printf( "INCR_WRAP" );
				break;
			case GLS_STENCIL_OP_ZFAIL_DECR_WRAP:
				idLib::Printf( "DECR_WRAP" );
				break;
			default:
				idLib::Printf( "NA" );
				break;
		}
		idLib::Printf( ", OpPass=" );
		switch( bits & GLS_STENCIL_OP_PASS_BITS )
		{
			case GLS_STENCIL_OP_PASS_KEEP:
				idLib::Printf( "KEEP" );
				break;
			case GLS_STENCIL_OP_PASS_ZERO:
				idLib::Printf( "ZERO" );
				break;
			case GLS_STENCIL_OP_PASS_REPLACE:
				idLib::Printf( "REPLACE" );
				break;
			case GLS_STENCIL_OP_PASS_INCR:
				idLib::Printf( "INCR" );
				break;
			case GLS_STENCIL_OP_PASS_DECR:
				idLib::Printf( "DECR" );
				break;
			case GLS_STENCIL_OP_PASS_INVERT:
				idLib::Printf( "INVERT" );
				break;
			case GLS_STENCIL_OP_PASS_INCR_WRAP:
				idLib::Printf( "INCR_WRAP" );
				break;
			case GLS_STENCIL_OP_PASS_DECR_WRAP:
				idLib::Printf( "DECR_WRAP" );
				break;
			default:
				idLib::Printf( "NA" );
				break;
		}
		idLib::Printf( ", mask=%llu, ref=%llu\n", mask, ref );
	};

	uint32 mask = uint32( ( stateBits & GLS_STENCIL_FUNC_MASK_BITS ) >> GLS_STENCIL_FUNC_MASK_SHIFT );
	uint32 ref = uint32( ( stateBits & GLS_STENCIL_FUNC_REF_BITS ) >> GLS_STENCIL_FUNC_REF_SHIFT );
	if( stateBits & GLS_SEPARATE_STENCIL )
	{
		printStencil( STENCIL_FACE_FRONT, ( stateBits & GLS_STENCIL_FRONT_OPS ), mask, ref );
		printStencil( STENCIL_FACE_BACK, ( ( stateBits & GLS_STENCIL_BACK_OPS ) >> 12 ), mask, ref );
	}
	else
	{
		printStencil( STENCIL_FACE_NUM, stateBits, mask, ref );
	}
}

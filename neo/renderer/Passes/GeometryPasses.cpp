/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2022 Stephen Pridham

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
#include <precompiled.h>
#pragma hdrstop

#include "renderer/RenderCommon.h"

#include "GeometryPasses.h"

struct BufferGroup
{
	nvrhi::IBuffer* vertexBuffer = nullptr;
	nvrhi::IBuffer* indexBuffer = nullptr;
	nvrhi::IBuffer* instanceBuffer = nullptr;

	bool operator==( BufferGroup& other ) const
	{
		return vertexBuffer == other.vertexBuffer &&
			   indexBuffer == other.indexBuffer &&
			   instanceBuffer == other.instanceBuffer;
	}

	bool operator!= ( BufferGroup& other ) const
	{
		return !( *this == other );
	}
};

void RenderView( nvrhi::ICommandList* commandList, const viewDef_t* view, const viewDef_t* prevView, nvrhi::IFramebuffer* framebuffer, idGeometryPass& pass, idGeometryPassContext& passContext, bool materialEvents )
{
	pass.SetupView( passContext, commandList, view, prevView );

	const idMaterial* lastMaterial = nullptr;
	const viewEntity_t* lastSpace = nullptr;
	nvrhi::RasterCullMode lastCullMode = nvrhi::RasterCullMode::Back;

	bool drawMaterial = true;
	bool stateValid = false;

	const idMaterial* eventMaterial = nullptr;

	nvrhi::GraphicsState graphicsState;
	graphicsState.framebuffer = framebuffer;
	nvrhi::Viewport viewport{ ( float )view->viewport.x1,
							  ( float )view->viewport.x2 + 1,
							  ( float )view->viewport.y1,
							  ( float )view->viewport.y2 + 1,
							  view->viewport.zmin,
							  view->viewport.zmax };
	graphicsState.viewport.addViewportAndScissorRect( viewport );

	nvrhi::DrawArguments currentDraw;
	currentDraw.instanceCount = 0;

	auto flushDraw = [commandList, materialEvents, &graphicsState, &currentDraw, &eventMaterial, &pass, &passContext]( const idMaterial * material )
	{
		if( currentDraw.instanceCount == 0 )
		{
			return;
		}

		if( materialEvents && material != eventMaterial )
		{
			if( eventMaterial )
			{
				renderLog.CloseBlock();
			}

			if( !material->GetName() )
			{
				eventMaterial = nullptr;
			}
			else
			{
				renderLog.OpenBlock( material->GetName(), colorMdGrey );
				eventMaterial = material;
			}
		}

		pass.SetPushConstants( passContext, commandList, graphicsState, currentDraw );

		commandList->draw( currentDraw );
		currentDraw.instanceCount = 0;
	};

	int geoNum = 0;
	const int numDrawSurfs = view->numDrawSurfs;

	for( int surfNum = 0; surfNum < numDrawSurfs; surfNum++ )
	{
		drawSurf_t* surf = view->drawSurfs[surfNum];
		const idMaterial* shader = surf->material;

		++geoNum;

		if( surf->space != lastSpace )
		{
			geoNum = 0;
		}

		lastSpace = surf->space;

		if( !surf->frontEndGeo )
		{
			continue;
		}

		if( !shader )
		{
			continue;
		}

		if( surf->material->Coverage() == MC_TRANSLUCENT )
		{
			// shouldn't be added to the depth buffer
			continue;
		}

		const float* regs = surf->shaderRegisters;

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
			continue;
		}

		nvrhi::RasterCullMode cullMode = GetCullMode( surf );

		if( lastCullMode != cullMode )
		{
			pass.SetupMaterial( passContext, surf, cullMode, graphicsState );
			lastCullMode = cullMode;
			stateValid = false;
		}

		if( !stateValid )
		{
			commandList->setGraphicsState( graphicsState );
			stateValid = true;
		}

		bufferView_t instanceBuffer;
		vertexCache.GetInstanceBuffer( surf->instanceCache, &instanceBuffer );

		nvrhi::DrawArguments args;
		args.vertexCount = surf->numIndexes;
		args.instanceCount = 1;
		currentDraw = args;
		idVec2i constants = idVec2i( instanceBuffer.offset / instanceBuffer.size, geoNum );
		commandList->setPushConstants( &constants, sizeof( idVec2i ) );
		flushDraw( shader );
	}

	lastSpace = nullptr;

	if( materialEvents && eventMaterial )
	{
		commandList->endMarker();
	}
}

nvrhi::RasterCullMode GetCullMode( drawSurf_t* surf )
{
	switch( surf->material->GetCullType() )
	{
		case CT_FRONT_SIDED:
			return nvrhi::RasterCullMode::Front;
		case CT_BACK_SIDED:
			return nvrhi::RasterCullMode::Back;
		case CT_TWO_SIDED:
			return nvrhi::RasterCullMode::None;
		default:
			common->Warning( "Invalid cull mode" );
			return nvrhi::RasterCullMode::None;
	}
}

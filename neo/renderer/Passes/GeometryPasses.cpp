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
	BufferGroup lastBuffers;
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

		commandList->drawIndexed( currentDraw );
		currentDraw.instanceCount = 0;
	};

	int surfNum = 0;
	int numDrawSurfs = view->numDrawSurfs;
	const viewEntity_t* currentSpace = nullptr;

	for( ; surfNum < numDrawSurfs; surfNum++ )
	{
		drawSurf_t* item = view->drawSurfs[surfNum];

		if( !item->material )
		{
			continue;
		}

		idVertexBuffer vertexBuffer;
		if( item->skinnedCache != 0 )
		{
			vertexCache.GetSkinnedVertexBuffer( item->skinnedCache, &vertexBuffer );
		}
		else
		{
			vertexCache.GetVertexBuffer( item->ambientCache, &vertexBuffer );
		}

		idIndexBuffer indexBuffer;
		vertexCache.GetIndexBuffer( item->indexCache, &indexBuffer );
		idVertexBuffer instanceBuffer;
		vertexCache.GetInstanceBuffer( item->space->instanceCache, &instanceBuffer );

		BufferGroup currBufferGroup =
		{
			vertexBuffer.GetAPIObject(),
			indexBuffer.GetAPIObject(),
			instanceBuffer.GetAPIObject()
		};

		bool newBuffers = currBufferGroup != lastBuffers;
		bool newMaterial = item->material != lastMaterial;

		if( item->space != currentSpace )
		{
			currentSpace = item->space;
		}

		if( newBuffers || newMaterial )
		{
			flushDraw( lastMaterial );
		}

		if( newBuffers || true )
		{
			pass.SetupInputBuffers( passContext, item, graphicsState );

			lastBuffers = currBufferGroup;
			stateValid = false;
		}

		nvrhi::RasterCullMode cullMode = lastCullMode;

		switch( item->material->GetCullType() )
		{
			case CT_FRONT_SIDED:
				cullMode = nvrhi::RasterCullMode::Front;
				break;
			case CT_BACK_SIDED:
				cullMode = nvrhi::RasterCullMode::Back;
				break;
			case CT_TWO_SIDED:
				cullMode = nvrhi::RasterCullMode::None;
				break;
			default:
				common->Warning( "Invalid cull mode" );
		}

		if( newMaterial )
		{
			drawMaterial = pass.SetupMaterial( passContext, item, cullMode, graphicsState );

			lastMaterial = item->material;
			lastCullMode = cullMode;
			stateValid = false;
		}

		if( drawMaterial )
		{
			if( !stateValid )
			{
				commandList->setGraphicsState( graphicsState );
				stateValid = true;
			}

			nvrhi::DrawArguments args;
			args.vertexCount = item->numIndexes;
			args.instanceCount = 1;
			args.startVertexLocation = vertexBuffer.GetOffset() / sizeof( idDrawVert );
			args.startIndexLocation = indexBuffer.GetOffset() / sizeof( triIndex_t );
			args.startInstanceLocation = instanceBuffer.GetOffset() / sizeof( idInstanceData );

			if( currentDraw.instanceCount > 0 &&
					currentDraw.startIndexLocation == args.startIndexLocation &&
					currentDraw.startInstanceLocation + currentDraw.instanceCount == args.startInstanceLocation )
			{
				currentDraw.instanceCount += 1;
			}
			else
			{
				flushDraw( item->material );

				currentDraw = args;
			}
		}
	}

	flushDraw( lastMaterial );

	if( materialEvents && eventMaterial )
	{
		commandList->endMarker();
	}
}

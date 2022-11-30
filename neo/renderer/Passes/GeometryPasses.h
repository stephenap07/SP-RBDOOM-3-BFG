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

#ifndef RENDERER_PASSES_GEOMETRYPASSES_H_
#define RENDERER_PASSES_GEOMETRYPASSES_H_

class idGeometryPassContext
{
};

class idGeometryPass
{
public:

	virtual void SetupView( idGeometryPassContext& abstractContext, nvrhi::ICommandList* commandList, const viewDef_t* view, const viewDef_t* viewPrev ) = 0;
	virtual bool SetupMaterial( idGeometryPassContext& abstractContext, drawSurf_t* drawSurf, nvrhi::RasterCullMode cullMode, nvrhi::GraphicsState& state ) = 0;
	virtual void SetupInputBuffers( idGeometryPassContext& abstractContext, drawSurf_t* drawSurf, nvrhi::GraphicsState& state ) = 0;
	virtual void SetPushConstants( idGeometryPassContext& abstractContext, nvrhi::ICommandList* commandList, nvrhi::GraphicsState& state, nvrhi::DrawArguments& args ) = 0;
	virtual ~idGeometryPass() = default;
};

void RenderView(
	nvrhi::ICommandList* commandList,
	const viewDef_t* view,
	const viewDef_t* prevView,
	nvrhi::IFramebuffer* framebuffer,
	idGeometryPass& pass,
	idGeometryPassContext& passContext,
	bool materialEvents = false );

#endif
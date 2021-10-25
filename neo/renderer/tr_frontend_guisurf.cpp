/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014 Robert Beckebans

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
#include "Model_local.h"

#include "RmlUi/Core/Context.h"

/*
==========================================================================================

GUI SURFACES

==========================================================================================
*/

/*
================
R_SurfaceToTextureAxis

Calculates two axis for the surface such that a point dotted against
the axis will give a 0.0 to 1.0 range in S and T when inside the gui surface
================
*/
void R_SurfaceToTextureAxis( const srfTriangles_t* tri, idVec3& origin, idVec3 axis[3] )
{
	// find the bounds of the texture
	idVec2 boundsMin( 999999.0f, 999999.0f );
	idVec2 boundsMax( -999999.0f, -999999.0f );
	for( int i = 0 ; i < tri->numVerts ; i++ )
	{
		const idVec2 uv = tri->verts[i].GetTexCoord();
		boundsMin.x = Min( uv.x, boundsMin.x );
		boundsMax.x = Max( uv.x, boundsMax.x );
		boundsMin.y = Min( uv.y, boundsMin.y );
		boundsMax.y = Max( uv.y, boundsMax.y );
	}

	// use the floor of the midpoint as the origin of the
	// surface, which will prevent a slight misalignment
	// from throwing it an entire cycle off
	const idVec2 boundsOrg( floor( ( boundsMin.x + boundsMax.x ) * 0.5f ), floor( ( boundsMin.y + boundsMax.y ) * 0.5f ) );

	// determine the world S and T vectors from the first drawSurf triangle

	// RB: added check wether GPU skinning is available at all
	const idJointMat* joints = ( tri->staticModelWithJoints != NULL && r_useGPUSkinning.GetBool() && glConfig.gpuSkinningAvailable ) ? tri->staticModelWithJoints->jointsInverted : NULL;
	// RB end

	const idVec3 aXYZ = idDrawVert::GetSkinnedDrawVertPosition( tri->verts[ tri->indexes[0] ], joints );
	const idVec3 bXYZ = idDrawVert::GetSkinnedDrawVertPosition( tri->verts[ tri->indexes[1] ], joints );
	const idVec3 cXYZ = idDrawVert::GetSkinnedDrawVertPosition( tri->verts[ tri->indexes[2] ], joints );

	const idVec2 aST = tri->verts[ tri->indexes[0] ].GetTexCoord();
	const idVec2 bST = tri->verts[ tri->indexes[1] ].GetTexCoord();
	const idVec2 cST = tri->verts[ tri->indexes[2] ].GetTexCoord();

	float d0[5];
	d0[0] = bXYZ[0] - aXYZ[0];
	d0[1] = bXYZ[1] - aXYZ[1];
	d0[2] = bXYZ[2] - aXYZ[2];
	d0[3] = bST.x - aST.x;
	d0[4] = bST.y - aST.y;

	float d1[5];
	d1[0] = cXYZ[0] - aXYZ[0];
	d1[1] = cXYZ[1] - aXYZ[1];
	d1[2] = cXYZ[2] - aXYZ[2];
	d1[3] = cST.x - aST.x;
	d1[4] = cST.y - aST.y;

	const float area = d0[3] * d1[4] - d0[4] * d1[3];
	if( area == 0.0f )
	{
		axis[0].Zero();
		axis[1].Zero();
		axis[2].Zero();
		return;	// degenerate
	}
	const float inva = 1.0f / area;

	axis[0][0] = ( d0[0] * d1[4] - d0[4] * d1[0] ) * inva;
	axis[0][1] = ( d0[1] * d1[4] - d0[4] * d1[1] ) * inva;
	axis[0][2] = ( d0[2] * d1[4] - d0[4] * d1[2] ) * inva;

	axis[1][0] = ( d0[3] * d1[0] - d0[0] * d1[3] ) * inva;
	axis[1][1] = ( d0[3] * d1[1] - d0[1] * d1[3] ) * inva;
	axis[1][2] = ( d0[3] * d1[2] - d0[2] * d1[3] ) * inva;

	idPlane plane;
	plane.FromPoints( aXYZ, bXYZ, cXYZ );
	axis[2][0] = plane[0];
	axis[2][1] = plane[1];
	axis[2][2] = plane[2];

	// take point 0 and project the vectors to the texture origin
	VectorMA( aXYZ, boundsOrg.x - aST.x, axis[0], origin );
	VectorMA( origin, boundsOrg.y - aST.y, axis[1], origin );
}

/*
=================
R_RenderGuiSurf

Create a texture space on the given surface and
call the GUI generator to create quads for it.
=================
*/
static void R_RenderGuiSurf( idUserInterface* gui, const drawSurf_t* drawSurf )
{
	SCOPED_PROFILE_EVENT( "R_RenderGuiSurf" );

	// for testing the performance hit
	if( r_skipGuiShaders.GetInteger() == 1 )
	{
		return;
	}

	// don't allow an infinite recursion loop
	if( tr.guiRecursionLevel == 4 )
	{
		return;
	}

	tr.pc.c_guiSurfs++;

	// create the new matrix to draw on this surface
	idVec3 origin, axis[3];
	R_SurfaceToTextureAxis( drawSurf->frontEndGeo, origin, axis );

	float guiModelMatrix[16];
	float modelMatrix[16];

	guiModelMatrix[0 * 4 + 0] = axis[0][0] * ( 1.0f / SCREEN_WIDTH );
	guiModelMatrix[1 * 4 + 0] = axis[1][0] * ( 1.0f / SCREEN_HEIGHT );
	guiModelMatrix[2 * 4 + 0] = axis[2][0];
	guiModelMatrix[3 * 4 + 0] = origin[0];

	guiModelMatrix[0 * 4 + 1] = axis[0][1] * ( 1.0f / SCREEN_WIDTH );
	guiModelMatrix[1 * 4 + 1] = axis[1][1] * ( 1.0f / SCREEN_HEIGHT );
	guiModelMatrix[2 * 4 + 1] = axis[2][1];
	guiModelMatrix[3 * 4 + 1] = origin[1];

	guiModelMatrix[0 * 4 + 2] = axis[0][2] * ( 1.0f / SCREEN_WIDTH );
	guiModelMatrix[1 * 4 + 2] = axis[1][2] * ( 1.0f / SCREEN_HEIGHT );

	guiModelMatrix[2 * 4 + 2] = axis[2][2];
	guiModelMatrix[3 * 4 + 2] = origin[2];

	guiModelMatrix[0 * 4 + 3] = 0.0f;
	guiModelMatrix[1 * 4 + 3] = 0.0f;
	guiModelMatrix[2 * 4 + 3] = 0.0f;
	guiModelMatrix[3 * 4 + 3] = 1.0f;

	R_MatrixMultiply( guiModelMatrix, drawSurf->space->modelMatrix, modelMatrix );

	tr.guiRecursionLevel++;

	// call the gui, which will call the 2D drawing functions
	tr.guiModel->Clear();
	gui->Redraw( tr.viewDef->renderView.time[0] );
	tr.guiModel->EmitToCurrentView( modelMatrix, drawSurf->space->weaponDepthHack );
	tr.guiModel->Clear();

	tr.guiRecursionLevel--;
}

/*
=================
R_RenderRmlSurf

Create a texture space on the given surface and
call the GUI generator to create quads for it.
=================
*/
static triIndex_t quadPicIndexes[6] = { 3, 0, 2, 2, 0, 1 };
static void R_RenderRmlSurf( RmlUserInterface* gui, const drawSurf_t* drawSurf )
{
	SCOPED_PROFILE_EVENT( "R_RenderRmlSurf" );

	// for testing the performance hit
	if( r_skipGuiShaders.GetInteger() == 1 )
	{
		return;
	}

	// don't allow an infinite recursion loop
	if( tr.guiRecursionLevel == 4 )
	{
		return;
	}

	tr.pc.c_guiSurfs++;

	bool renderToTexture = false;

	for( int i = 0; i < drawSurf->material->GetNumStages(); i++ )
	{
		shaderStage_t* stage = const_cast<shaderStage_t*>( drawSurf->material->GetStage( i ) );

		if( stage->texture.dynamic == DI_GUI_RENDER )
		{
			tr.guiRecursionLevel++;

			stage->texture.dynamicFrameCount = tr.frameCount;

			// Call the gui, which will call the 2D drawing functions
			//tr.CropRenderSize( stage->texture.width, stage->texture.height );
			gui->SetUseScreenResolution( false );
			gui->SetSize( stage->texture.width, stage->texture.height );
			//tr.viewDef->targetRender = globalFramebuffers.glowFBO[0];
			gui->Redraw( tr.viewDef->renderView.time[0] / 1000 );
			tr.guiModel->EmitFullScreen( &stage->texture );
			tr.guiModel->Clear();
			//tr.UnCrop();

			tr.guiRecursionLevel--;

			renderToTexture = true;
		}

		if( stage->texture.dynamic == DI_RENDER_TARGET )
		{
			stage->texture.dynamicFrameCount = tr.frameCount;

			viewDef_t* viewDef = ( viewDef_t* )R_ClearedFrameAlloc( sizeof( *viewDef ), FRAME_ALLOC_VIEW_DEF );
			viewDef->is2Dgui = true;

			//tr.CropRenderSize(stage->texture.width, stage->texture.height);
			{
				viewDef->viewport.x1 = 0;
				viewDef->viewport.y1 = 0;
				viewDef->viewport.x2 = stage->texture.width;
				viewDef->viewport.y2 = stage->texture.height;
			}

			//tr.GetCroppedViewport(&viewDef->viewport);

			viewDef->scissor.x1 = 0;
			viewDef->scissor.y1 = 0;
			viewDef->scissor.x2 = viewDef->viewport.x2 - viewDef->viewport.x1;
			viewDef->scissor.y2 = viewDef->viewport.y2 - viewDef->viewport.y1;

			// RB: IMPORTANT - the projectionMatrix has a few changes to make it work with Vulkan
			viewDef->projectionMatrix[0 * 4 + 0] = 2.0f / stage->texture.width;
			viewDef->projectionMatrix[0 * 4 + 1] = 0.0f;
			viewDef->projectionMatrix[0 * 4 + 2] = 0.0f;
			viewDef->projectionMatrix[0 * 4 + 3] = 0.0f;

			viewDef->projectionMatrix[1 * 4 + 0] = 0.0f;
#if defined(USE_VULKAN)
			viewDef->projectionMatrix[1 * 4 + 1] = 2.0f / renderSystem->GetVirtualHeight();
#else
			viewDef->projectionMatrix[1 * 4 + 1] = -2.0f / stage->texture.height;
#endif
			viewDef->projectionMatrix[1 * 4 + 2] = 0.0f;
			viewDef->projectionMatrix[1 * 4 + 3] = 0.0f;

			viewDef->projectionMatrix[2 * 4 + 0] = 0.0f;
			viewDef->projectionMatrix[2 * 4 + 1] = 0.0f;
			viewDef->projectionMatrix[2 * 4 + 2] = -1.0f;
			viewDef->projectionMatrix[2 * 4 + 3] = 0.0f;

			viewDef->projectionMatrix[3 * 4 + 0] = -1.0f; // RB: was -2.0f
#if defined(USE_VULKAN)
			viewDef->projectionMatrix[3 * 4 + 1] = -1.0f;
#else
			viewDef->projectionMatrix[3 * 4 + 1] = 1.0f;
#endif
			viewDef->projectionMatrix[3 * 4 + 2] = 0.0f; // RB: was 1.0f
			viewDef->projectionMatrix[3 * 4 + 3] = 1.0f;

			// make a tech5 renderMatrix for faster culling
			idRenderMatrix::Transpose( *( idRenderMatrix* )viewDef->projectionMatrix, viewDef->projectionRenderMatrix );

			viewDef->worldSpace.modelMatrix[0 * 4 + 0] = 1.0f;
			viewDef->worldSpace.modelMatrix[1 * 4 + 1] = 1.0f;
			viewDef->worldSpace.modelMatrix[2 * 4 + 2] = 1.0f;
			viewDef->worldSpace.modelMatrix[3 * 4 + 3] = 1.0f;

			viewDef->worldSpace.modelViewMatrix[0 * 4 + 0] = 1.0f;
			viewDef->worldSpace.modelViewMatrix[1 * 4 + 1] = 1.0f;
			viewDef->worldSpace.modelViewMatrix[2 * 4 + 2] = 1.0f;
			viewDef->worldSpace.modelViewMatrix[3 * 4 + 3] = 1.0f;

			viewDef->maxDrawSurfs = 1;
			viewDef->drawSurfs = ( drawSurf_t** )R_FrameAlloc( viewDef->maxDrawSurfs * sizeof( viewDef->drawSurfs[0] ), FRAME_ALLOC_DRAW_SURFACE_POINTER );
			viewDef->numDrawSurfs = 0;

			int shaderTime = tr.frameShaderTime * 1000;
			viewDef->renderView.time[0] = shaderTime;
			viewDef->renderView.time[1] = shaderTime;

			// allocate vertices

			vertCacheHandle_t vertexBlock = vertexCache.AllocVertex( NULL, 4 );
			vertCacheHandle_t indexBlock = vertexCache.AllocIndex( NULL, 6 );
			idDrawVert* vertexPointer = ( idDrawVert* )vertexCache.MappedVertexBuffer( vertexBlock );
			triIndex_t* indexPointer = ( triIndex_t* )vertexCache.MappedIndexBuffer( indexBlock );

			for( int i = 0; i < 6; i++ )
			{
				indexPointer[i] = quadPicIndexes[i];
			}

			{
				auto currentColorNativeBytesOrder = LittleLong( PackColor( colorWhite ) );
				ALIGNTYPE16 idDrawVert localVerts[4];

				float x1 = 0.0f;
				float y1 = 0.0f;
				float x2 = stage->texture.width;
				float y2 = stage->texture.height;

				float s1 = 0.0f;
				float t1 = 0.0f;
				float s2 = 1.0f;
				float t2 = 1.0f;

				// top left
				localVerts[0].Clear();
				localVerts[0].xyz[0] = x1;
				localVerts[0].xyz[1] = y2;
				localVerts[0].xyz[2] = 0.0f;
				localVerts[0].SetTexCoord( s1, t2 );
				localVerts[0].SetNativeOrderColor( currentColorNativeBytesOrder );
				localVerts[0].ClearColor2();

				// top right
				localVerts[1].Clear();
				localVerts[1].xyz[0] = x2;
				localVerts[1].xyz[1] = y2;
				localVerts[1].xyz[2] = 0.0f;
				localVerts[1].SetTexCoord( s2, t2 );
				localVerts[1].SetNativeOrderColor( currentColorNativeBytesOrder );
				localVerts[1].ClearColor2();

				// bottom right
				localVerts[2].Clear();
				localVerts[2].xyz[0] = x2;
				localVerts[2].xyz[1] = y1;
				localVerts[2].xyz[2] = 0.0f;
				localVerts[2].SetTexCoord( s2, t1 );
				localVerts[2].SetNativeOrderColor( currentColorNativeBytesOrder );
				localVerts[2].ClearColor2();

				// bottom left
				localVerts[3].Clear();
				localVerts[3].xyz[0] = x1;
				localVerts[3].xyz[1] = y1;
				localVerts[3].xyz[2] = 0.0f;
				localVerts[3].SetTexCoord( s1, t1 );
				localVerts[3].SetNativeOrderColor( currentColorNativeBytesOrder );
				localVerts[3].ClearColor2();

				WriteDrawVerts16( vertexPointer, localVerts, 4 );
			}

			//WriteDrawVerts16(vertexPointer, tr.unitSquareTriangles->verts, 4);

			viewEntity_t* guiSpace = ( viewEntity_t* )R_ClearedFrameAlloc( sizeof( *guiSpace ), FRAME_ALLOC_VIEW_ENTITY );

			memcpy( guiSpace->modelMatrix, viewDef->worldSpace.modelMatrix, sizeof( guiSpace->modelMatrix ) );
			memcpy( guiSpace->modelViewMatrix, viewDef->worldSpace.modelViewMatrix, sizeof( guiSpace->modelViewMatrix ) );
			guiSpace->weaponDepthHack = false;
			guiSpace->isGuiSurface = true;

			//---------------------------
			// make a tech5 renderMatrix
			//---------------------------
			idRenderMatrix viewMat;
			idRenderMatrix::Transpose( *( idRenderMatrix* )viewDef->worldSpace.modelViewMatrix, viewMat );
			idRenderMatrix::Multiply( viewDef->projectionRenderMatrix, viewMat, guiSpace->mvp );

			drawSurf_t* newSurf = ( drawSurf_t* )R_FrameAlloc( sizeof( *newSurf ), FRAME_ALLOC_DRAW_SURFACE );
			newSurf->numIndexes = 6; // for a fullscreen quad.
			newSurf->ambientCache = vertexBlock;
			// build a vertCacheHandle_t that points inside the allocated block
			newSurf->indexCache = indexBlock + ( ( int64 )( 0 ) << VERTCACHE_OFFSET_SHIFT );
			newSurf->shadowCache = 0;
			newSurf->jointCache = 0;
			newSurf->frontEndGeo = NULL;
			newSurf->space = guiSpace;
			newSurf->material = stage->texture.renderTargetMaterial;
			newSurf->extraGLState = newSurf->extraGLState;
			newSurf->scissorRect = viewDef->scissor;
			newSurf->sort = newSurf->material->GetSort();
			newSurf->renderZFail = 0;

			// process the shader expressions for conditionals / color / texcoords
			const float* constRegs = newSurf->material->ConstantRegisters();
			if( constRegs )
			{
				// shader only uses constant values
				newSurf->shaderRegisters = constRegs;
			}
			else
			{
				float* regs = ( float* )R_FrameAlloc( newSurf->material->GetNumRegisters() * sizeof( float ), FRAME_ALLOC_SHADER_REGISTER );
				newSurf->shaderRegisters = regs;

				float localShaderParms[MAX_ENTITY_SHADER_PARMS];
				for( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ )
				{
					localShaderParms[i] = 1.0f;
				}

				newSurf->material->EvaluateRegisters( regs, localShaderParms, viewDef->renderView.shaderParms, viewDef->renderView.time[1] * 0.001f, NULL );
			}

			R_LinkDrawSurfToView( newSurf, viewDef );

			viewDef->superView = tr.viewDef;

			// TODO(Stephen) make this configurable in the renderTarget material
			viewDef->targetRender = globalFramebuffers.glowFBO[1];

			stage->texture.dynamicFrameCount = tr.frameCount;
			stage->texture.image = globalImages->glowImage[1];

			R_AddDrawViewCmd( viewDef, true );

			//tr.UnCrop();

			renderToTexture = true;
		}
	}

	if( renderToTexture )
	{
		return;
	}

	// create the new matrix to draw on this surface
	idVec3 origin, axis[3];
	R_SurfaceToTextureAxis( drawSurf->frontEndGeo, origin, axis );

	float guiModelMatrix[16];
	float modelMatrix[16];

	const float screenWidth = gui->GetScreenSize().x;
	const float screenHeight = gui->GetScreenSize().y;

	guiModelMatrix[0 * 4 + 0] = axis[0][0] * ( 1.0f / screenWidth );
	guiModelMatrix[1 * 4 + 0] = axis[1][0] * ( 1.0f / screenHeight );
	guiModelMatrix[2 * 4 + 0] = axis[2][0];
	guiModelMatrix[3 * 4 + 0] = origin[0];

	guiModelMatrix[0 * 4 + 1] = axis[0][1] * ( 1.0f / screenWidth );
	guiModelMatrix[1 * 4 + 1] = axis[1][1] * ( 1.0f / screenHeight );
	guiModelMatrix[2 * 4 + 1] = axis[2][1];
	guiModelMatrix[3 * 4 + 1] = origin[1];

	guiModelMatrix[0 * 4 + 2] = axis[0][2] * ( 1.0f / screenWidth );
	guiModelMatrix[1 * 4 + 2] = axis[1][2] * ( 1.0f / screenHeight );

	guiModelMatrix[2 * 4 + 2] = axis[2][2];
	guiModelMatrix[3 * 4 + 2] = origin[2];

	guiModelMatrix[0 * 4 + 3] = 0.0f;
	guiModelMatrix[1 * 4 + 3] = 0.0f;
	guiModelMatrix[2 * 4 + 3] = 0.0f;
	guiModelMatrix[3 * 4 + 3] = 1.0f;

	R_MatrixMultiply( guiModelMatrix, drawSurf->space->modelMatrix, modelMatrix );

	tr.guiRecursionLevel++;

	// call the gui, which will call the 2D drawing functions
	tr.guiModel->Clear();
	gui->Redraw( tr.viewDef->renderView.time[0] / 1000 );
	tr.guiModel->EmitToCurrentView( modelMatrix, drawSurf->space->weaponDepthHack );
	tr.guiModel->Clear();

	tr.guiRecursionLevel--;
}

/*
================
R_AddInGameGuis
================
*/
void R_AddInGameGuis( const drawSurf_t* const drawSurfs[], const int numDrawSurfs )
{
	SCOPED_PROFILE_EVENT( "R_AddInGameGuis" );

	// check for gui surfaces
	for( int i = 0; i < numDrawSurfs; i++ )
	{
		const drawSurf_t* drawSurf = drawSurfs[i];
		idUserInterface* gui = drawSurf->material->GlobalGui();

		int guiNum = drawSurf->material->GetEntityGui() - 1;
		if( guiNum >= 0 && guiNum < MAX_RENDERENTITY_GUI )
		{
			if( drawSurf->space->entityDef != NULL )
			{
				gui = drawSurf->space->entityDef->parms.gui[guiNum];
			}
		}

		if( gui == NULL )
		{
			continue;
		}

		idBounds ndcBounds;
		if( !R_PreciseCullSurface( drawSurf, ndcBounds ) )
		{
			// did we ever use this to forward an entity color to a gui that didn't set color?
			//	memcpy( tr.guiShaderParms, shaderParms, sizeof( tr.guiShaderParms ) );
			R_RenderGuiSurf( gui, drawSurf );
		}
	}

	// check for rml surfaces
	for( int i = 0; i < numDrawSurfs; i++ )
	{
		const drawSurf_t* drawSurf = drawSurfs[i];
		RmlUserInterface* gui = drawSurf->material->RmlGui();

		int guiNum = drawSurf->material->GetRmlEntityGui() - 1;
		if( guiNum >= 0 && guiNum < MAX_RENDERENTITY_GUI )
		{
			if( drawSurf->space->entityDef != NULL )
			{
				gui = drawSurf->space->entityDef->parms.rml[guiNum];
			}
		}

		if( gui == NULL )
		{
			continue;
		}

		idBounds ndcBounds;
		if( !R_PreciseCullSurface( drawSurf, ndcBounds ) )
		{
			R_RenderRmlSurf( gui, drawSurf );
		}
	}
}

/*
================,
R_ReloadGuis_f

Reloads any guis that have had their file timestamps changed.
An optional "all" parameter will cause all models to reload, even
if they are not out of date.

Should we also reload the map models?
================
*/
void R_ReloadGuis_f( const idCmdArgs& args )
{
	bool all;

	if( !idStr::Icmp( args.Argv( 1 ), "all" ) )
	{
		all = true;
		common->Printf( "Reloading all gui files...\n" );
	}
	else
	{
		all = false;
		common->Printf( "Checking for changed gui files...\n" );
	}

	uiManager->Reload( all );
}

/*
================,
R_ListGuis_f
================
*/
void R_ListGuis_f( const idCmdArgs& args )
{
	uiManager->ListGuis();
}

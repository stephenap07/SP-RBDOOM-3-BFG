/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2015 Robert Beckebans

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

#pragma hdrstop
#include "precompiled.h"

#include "../tr_local.h"
#include "../../framework/Common_local.h"

#include <openvr.h>

idCVar r_drawFlickerBox( "r_drawFlickerBox", "0", CVAR_RENDERER | CVAR_BOOL, "visual test for dropping frames" );
idCVar stereoRender_warp( "stereoRender_warp", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "use the optical warping renderprog instead of stereoDeGhost" );
idCVar stereoRender_warpStrength( "stereoRender_warpStrength", "1.45", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_FLOAT, "amount of pre-distortion" );

idCVar stereoRender_warpCenterX( "stereoRender_warpCenterX", "0.5", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "center for left eye, right eye will be 1.0 - this" );
idCVar stereoRender_warpCenterY( "stereoRender_warpCenterY", "0.5", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "center for both eyes" );
idCVar stereoRender_warpParmZ( "stereoRender_warpParmZ", "0", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "development parm" );
idCVar stereoRender_warpParmW( "stereoRender_warpParmW", "0", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "development parm" );
idCVar stereoRender_warpTargetFraction( "stereoRender_warpTargetFraction", "1.0", CVAR_RENDERER | CVAR_FLOAT | CVAR_ARCHIVE, "fraction of half-width the through-lens view covers" );

idCVar r_showSwapBuffers( "r_showSwapBuffers", "0", CVAR_BOOL, "Show timings from GL_BlockingSwapBuffers" );
idCVar r_syncEveryFrame( "r_syncEveryFrame", "1", CVAR_BOOL, "Don't let the GPU buffer execution past swapbuffers" );

static int		swapIndex;		// 0 or 1 into renderSync
static GLsync	renderSync[2];

void GLimp_SwapBuffers();
void RB_SetMVP( const idRenderMatrix& mvp );

/*
============================================================================

RENDER BACK END THREAD FUNCTIONS

============================================================================
*/

/*
=============
RB_DrawFlickerBox
=============
*/
static void RB_DrawFlickerBox()
{
	if( !r_drawFlickerBox.GetBool() )
	{
		return;
	}
	if( tr.frameCount & 1 )
	{
		glClearColor( 1, 0, 0, 1 );
	}
	else
	{
		glClearColor( 0, 1, 0, 1 );
	}
	glScissor( 0, 0, 256, 256 );
	glClear( GL_COLOR_BUFFER_BIT );
}

/*
=============
RB_SetBuffer
=============
*/
static void	RB_SetBuffer( const void* data )
{
	// see which draw buffer we want to render the frame to
	
	const setBufferCommand_t* cmd = ( const setBufferCommand_t* )data;
	
	RENDERLOG_PRINTF( "---------- RB_SetBuffer ---------- to buffer # %d\n", cmd->buffer );
	
	GL_Scissor( 0, 0, tr.GetWidth(), tr.GetHeight() );
	
	// clear screen for debugging
	// automatically enable this with several other debug tools
	// that might leave unrendered portions of the screen
	if( r_clear.GetFloat() || idStr::Length( r_clear.GetString() ) != 1 || r_singleArea.GetBool() || r_showOverDraw.GetBool() )
	{
		float c[3];
		if( sscanf( r_clear.GetString(), "%f %f %f", &c[0], &c[1], &c[2] ) == 3 )
		{
			GL_Clear( true, false, false, 0, c[0], c[1], c[2], 1.0f, true );
		}
		else if( r_clear.GetInteger() == 2 )
		{
			GL_Clear( true, false, false, 0, 0.0f, 0.0f,  0.0f, 1.0f, true );
		}
		else if( r_showOverDraw.GetBool() )
		{
			GL_Clear( true, false, false, 0, 1.0f, 1.0f, 1.0f, 1.0f, true );
		}
		else
		{
			GL_Clear( true, false, false, 0, 0.4f, 0.0f, 0.25f, 1.0f, true );
		}
	}
}

/*
=============
GL_BlockingSwapBuffers

We want to exit this with the GPU idle, right at vsync
=============
*/
const void GL_BlockingSwapBuffers()
{
	RENDERLOG_PRINTF( "***************** GL_BlockingSwapBuffers *****************\n\n\n" );
	
	const int beforeFinish = Sys_Milliseconds();
	
	if( !glConfig.syncAvailable )
	{
		glFinish();
	}
	
	const int beforeSwap = Sys_Milliseconds();
	if( r_showSwapBuffers.GetBool() && beforeSwap - beforeFinish > 1 )
	{
		common->Printf( "%i msec to glFinish\n", beforeSwap - beforeFinish );
	}
	
	GLimp_SwapBuffers();
	
	if (glConfig.openVREnabled)
	{
		VR_PostSwap();
	}

	const int beforeFence = Sys_Milliseconds();
	if( r_showSwapBuffers.GetBool() && beforeFence - beforeSwap > 1 )
	{
		common->Printf( "%i msec to swapBuffers\n", beforeFence - beforeSwap );
	}
	
	if( glConfig.syncAvailable )
	{
		swapIndex ^= 1;
		
		if( glIsSync( renderSync[swapIndex] ) )
		{
			glDeleteSync( renderSync[swapIndex] );
		}
		// draw something tiny to ensure the sync is after the swap
		const int start = Sys_Milliseconds();
		glScissor( 0, 0, 1, 1 );
		glEnable( GL_SCISSOR_TEST );
		glClear( GL_COLOR_BUFFER_BIT );
		renderSync[swapIndex] = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
		const int end = Sys_Milliseconds();
		if( r_showSwapBuffers.GetBool() && end - start > 1 )
		{
			common->Printf( "%i msec to start fence\n", end - start );
		}
		
		GLsync	syncToWaitOn;
		if( r_syncEveryFrame.GetBool() )
		{
			syncToWaitOn = renderSync[swapIndex];
		}
		else
		{
			syncToWaitOn = renderSync[!swapIndex];
		}
		
		if( glIsSync( syncToWaitOn ) )
		{
			for( GLenum r = GL_TIMEOUT_EXPIRED; r == GL_TIMEOUT_EXPIRED; )
			{
				r = glClientWaitSync( syncToWaitOn, GL_SYNC_FLUSH_COMMANDS_BIT, 1000 * 1000 );
			}
		}
	}
	
	const int afterFence = Sys_Milliseconds();
	if( r_showSwapBuffers.GetBool() && afterFence - beforeFence > 1 )
	{
		common->Printf( "%i msec to wait on fence\n", afterFence - beforeFence );
	}
	
	const int64 exitBlockTime = Sys_Microseconds();
	
	static int64 prevBlockTime;
	if( r_showSwapBuffers.GetBool() && prevBlockTime )
	{
		const int delta = ( int )( exitBlockTime - prevBlockTime );
		common->Printf( "blockToBlock: %i\n", delta );
	}
	prevBlockTime = exitBlockTime;
}

/*
====================
R_MakeStereoRenderImage
====================
*/
static void R_MakeStereoRenderImage( idImage* image )
{
	idImageOpts	opts;
	opts.textureType = ( glConfig.multisamples > 0 ) ? TT_2D_MULTISAMPLE : TT_2D;
	opts.width = renderSystem->GetWidth();
	opts.height = renderSystem->GetHeight();
	opts.numLevels = 1;
	opts.format = FMT_RGBA8;
	opts.msaaSamples = glConfig.multisamples;
	image->AllocImage( opts, TF_LINEAR, TR_CLAMP );
}

static void R_MakeStereoRenderImage_NoMSAA( idImage* image )
{
	idImageOpts	opts;
	opts.textureType = TT_2D;
	opts.width = renderSystem->GetWidth();
	opts.height = renderSystem->GetHeight();
	opts.numLevels = 1;
	opts.format = FMT_RGBA8;
	opts.msaaSamples = 0;
	image->AllocImage( opts, TF_LINEAR, TR_CLAMP );
}

void R_DepthImage( idImage* image );

/*
====================
RB_StereoRenderExecuteBackEndCommands

Renders the draw list twice, with slight modifications for left eye / right eye
====================
*/
void RB_StereoRenderExecuteBackEndCommands( const emptyCommand_t* const allCmds )
{
	uint64 backEndStartTime = Sys_Microseconds();
	
	// If we are in a monoscopic context, this draws to the only buffer, and is
	// the same as GL_BACK.  In a quad-buffer stereo context, this is necessary
	// to prevent GL from forcing the rendering to go to both BACK_LEFT and
	// BACK_RIGHT at a performance penalty.
	// To allow stereo deghost processing, the views have to be copied to separate
	// textures anyway, so there isn't any benefit to rendering to BACK_RIGHT for
	// that eye.
	glDrawBuffer( GL_BACK_LEFT );

	int renderSystemWidth = renderSystem->GetWidth();
	int renderSystemHeight = renderSystem->GetHeight();
	bool stereoMultisample = ( glConfig.multisamples > 0 ) && !r_useHDR.GetBool();

	// create the stereoDepthImage if we haven't already
	static idImage* stereoDepthImage;
	if( stereoDepthImage == NULL )
	{
		stereoDepthImage = globalImages->ImageFromFunction( "_stereoDepth", R_DepthImage );
	}
	// resize stereoDepthImage if the main window has changed size
	if( stereoDepthImage->GetUploadWidth() != renderSystemWidth ||
		stereoDepthImage->GetUploadHeight() != renderSystemHeight )
	{
		stereoDepthImage->Resize( renderSystemWidth, renderSystemHeight );
	}
	// create the stereoMSAARenderImage if we haven't already
	static idImage* stereoMSAARenderImage;
	static Framebuffer * stereoMSAARenderFBO;
	if( stereoMultisample && !stereoMSAARenderImage )
	{
		if( !stereoMSAARenderImage )
		{
			stereoMSAARenderImage = globalImages->ImageFromFunction( "_stereoMSAARender", R_MakeStereoRenderImage );
			stereoMSAARenderFBO = new Framebuffer( "_stereoMSAAFBO", renderSystemWidth, renderSystemHeight );

			stereoMSAARenderFBO->Bind();

			stereoMSAARenderFBO->AddColorBuffer( GL_RGBA, 0, glConfig.multisamples );
			stereoMSAARenderFBO->AddDepthBuffer( GL_DEPTH24_STENCIL8, glConfig.multisamples );
	
			stereoMSAARenderFBO->AttachImage2D( stereoMSAARenderImage, 0 );
			stereoMSAARenderFBO->AttachImageDepth( stereoDepthImage );

			stereoMSAARenderFBO->Check();
		}
		// resize stereoDepthImage if the main window has changed size
		if( stereoMSAARenderImage->GetUploadWidth() != renderSystemWidth ||
			stereoMSAARenderImage->GetUploadHeight() != renderSystemHeight )
		{
			stereoMSAARenderImage->Resize( renderSystemWidth, renderSystemHeight );
			stereoMSAARenderFBO->Bind();
			stereoMSAARenderFBO->AttachImage2D( stereoMSAARenderImage, 0 );
			stereoMSAARenderFBO->AttachImageDepth( stereoDepthImage );
			stereoMSAARenderFBO->Check();
			stereoMSAARenderFBO->Resize( renderSystemWidth, renderSystemHeight );
		}
	}
	static idImage* stereoRenderImages[2];
	static Framebuffer * stereoRenderFBO[2];
	for( int i = 0; i < 2; i++ )
	{
		if( stereoRenderImages[i] == NULL )
		{
			stereoRenderImages[i] = globalImages->ImageFromFunction( va( "_stereoRender%i", i ), R_MakeStereoRenderImage_NoMSAA );
			stereoRenderFBO[i] = new Framebuffer( va( "_stereoFBO%i", i ), renderSystemWidth, renderSystemHeight );

			stereoRenderFBO[i]->Bind();

			stereoRenderFBO[i]->AddColorBuffer( GL_RGBA8, 0 );
			stereoRenderFBO[i]->AttachImage2D( stereoRenderImages[i], 0 );

			if( !stereoMultisample )
			{
				stereoRenderFBO[i]->AddDepthBuffer( GL_DEPTH24_STENCIL8 );
				stereoRenderFBO[i]->AttachImageDepth( stereoDepthImage );
			}

			stereoRenderFBO[i]->Check();
		}
		
		// resize the stereo render image if the main window has changed size
		if( stereoRenderImages[i]->GetUploadWidth() != renderSystemWidth ||
				stereoRenderImages[i]->GetUploadHeight() != renderSystemHeight )
		{
			stereoRenderImages[i]->Resize( renderSystemWidth, renderSystemHeight );

			stereoRenderFBO[i]->Bind();
			stereoRenderFBO[i]->AttachImage2D( stereoRenderImages[i], 0 );
			if( !stereoMultisample )
			{
				stereoRenderFBO[i]->AttachImageDepth( stereoDepthImage );
			}
			stereoRenderFBO[i]->Check();
			stereoRenderFBO[i]->Resize( renderSystemWidth, renderSystemHeight );
		}
	}
	
	// In stereoRender mode, the front end has generated two RC_DRAW_VIEW commands
	// with slightly different origins for each eye.
	
	// TODO: only do the copy after the final view has been rendered, not mirror subviews?
	
	// Render the 3D draw views from the screen origin so all the screen relative
	// texture mapping works properly, then copy the portion we are going to use
	// off to a texture.
	bool foundEye[2] = { false, false };
	
	for( int stereoEye = 1; stereoEye >= -1; stereoEye -= 2 )
	{
		// set up the target texture we will draw to
		const int targetEye = ( stereoEye == 1 ) ? 1 : 0;
		
		// Set the back end into a known default state to fix any stale render state issues
		if( stereoMultisample )
		{
			globalFramebuffers.currentStereoRenderFBO = stereoMSAARenderFBO;
			globalFramebuffers.currentStereoRenderNonMSAAFBO = stereoRenderFBO[ targetEye ];
		}
		else
		{
			globalFramebuffers.currentStereoRenderFBO = stereoRenderFBO[ targetEye ];
		}
		GL_SetDefaultState();
		renderProgManager.Unbind();
		renderProgManager.ZeroUniforms();
		
		for( const emptyCommand_t* cmds = allCmds; cmds != NULL; cmds = ( const emptyCommand_t* )cmds->next )
		{
			switch( cmds->commandId )
			{
				case RC_NOP:
					break;
				case RC_DRAW_VIEW_GUI:
				case RC_DRAW_VIEW_3D:
				{
					const drawSurfsCommand_t* const dsc = ( const drawSurfsCommand_t* )cmds;
					const viewDef_t&			eyeViewDef = *dsc->viewDef;
					
					if( eyeViewDef.renderView.viewEyeBuffer && eyeViewDef.renderView.viewEyeBuffer != stereoEye )
					{
						// this is the render view for the other eye
						continue;
					}
					
					foundEye[ targetEye ] = true;
					RB_DrawView( dsc, stereoEye );
					if( cmds->commandId == RC_DRAW_VIEW_GUI )
					{
					}
				}
				break;
				case RC_SET_BUFFER:
					RB_SetBuffer( cmds );
					break;
				case RC_COPY_RENDER:
				{
					const copyRenderCommand_t* crc = ( const copyRenderCommand_t* )cmds;
					if( crc->viewEyeBuffer && crc->viewEyeBuffer != stereoEye )
					{
						// avoid unnecessary copy
						continue;
					}
					RB_CopyRender( cmds );
					break;
				}
				case RC_POST_PROCESS:
				{
					postProcessCommand_t* cmd = ( postProcessCommand_t* )cmds;
					if( cmd->viewDef->renderView.viewEyeBuffer != stereoEye )
					{
						break;
					}
					RB_PostProcess( cmds );
				}
				break;
				default:
					common->Error( "RB_ExecuteBackEndCommands: bad commandId" );
					break;
			}
		}

		if( stereoMultisample )
		{
			glBindFramebuffer( GL_READ_FRAMEBUFFER, stereoMSAARenderFBO->GetFramebuffer() );
			glBindFramebuffer( GL_DRAW_FRAMEBUFFER, stereoRenderFBO[ targetEye ]->GetFramebuffer() );
			glBlitFramebuffer( 0, 0, renderSystemWidth, renderSystemHeight,
							   0, 0, renderSystemWidth, renderSystemHeight,
							   GL_COLOR_BUFFER_BIT,
							   GL_LINEAR );
		}
	}
	
	// perform the final compositing / warping / deghosting to the actual framebuffer(s)
	assert( foundEye[0] && foundEye[1] );
	
	globalFramebuffers.currentStereoRenderFBO = NULL;
	GL_SetDefaultState();
	
	RB_SetMVP( renderMatrix_identity );
	
	// If we are in quad-buffer pixel format but testing another 3D mode,
	// make sure we draw to both eyes.  This is likely to be sub-optimal
	// performance on most cards and drivers, but it is better than getting
	// a confusing, half-ghosted view.
	if( renderSystem->GetStereo3DMode() != STEREO3D_QUAD_BUFFER )
	{
		glDrawBuffer( GL_BACK );
	}
	
	GL_State( GLS_DEPTHFUNC_ALWAYS );
	GL_Cull( CT_TWO_SIDED );
	
	// We just want to do a quad pass - so make sure we disable any texgen and
	// set the texture matrix to the identity so we don't get anomalies from
	// any stale uniform data being present from a previous draw call
	const float texS[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	const float texT[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_S, texS );
	renderProgManager.SetRenderParm( RENDERPARM_TEXTUREMATRIX_T, texT );
	
	// disable any texgen
	const float texGenEnabled[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderProgManager.SetRenderParm( RENDERPARM_TEXGEN_0_ENABLED, texGenEnabled );
	
	renderProgManager.BindShader_Texture();
	GL_Color( 1, 1, 1, 1 );
	
	switch( renderSystem->GetStereo3DMode() )
	{
		case STEREO3D_QUAD_BUFFER:
			glDrawBuffer( GL_BACK_RIGHT );
			GL_SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			GL_SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			glDrawBuffer( GL_BACK_LEFT );
			GL_SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			GL_SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			break;
		case STEREO3D_HDMI_720:
			// HDMI 720P 3D
			GL_SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			GL_SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			GL_ViewportAndScissor( 0, 0, 1280, 720 );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			GL_SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			GL_SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			GL_ViewportAndScissor( 0, 750, 1280, 720 );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			// force the HDMI 720P 3D guard band to a constant color
			glScissor( 0, 720, 1280, 30 );
			glClear( GL_COLOR_BUFFER_BIT );
			break;
		default:
		case STEREO3D_SIDE_BY_SIDE:
			if( stereoRender_warp.GetBool() )
			{
				// this is the Rift warp
				// renderSystemWidth / GetHeight() have returned equal values (640 for initial Rift)
				// and we are going to warp them onto a symetric square region of each half of the screen
				
				renderProgManager.BindShader_StereoWarp();
				
				// clear the entire screen to black
				// we could be smart and only clear the areas we aren't going to draw on, but
				// clears are fast...
				glScissor( 0, 0, glConfig.nativeScreenWidth, glConfig.nativeScreenHeight );
				glClearColor( 0, 0, 0, 0 );
				glClear( GL_COLOR_BUFFER_BIT );
				
				// the size of the box that will get the warped pixels
				// With the 7" displays, this will be less than half the screen width
				const int pixelDimensions = ( glConfig.nativeScreenWidth >> 1 ) * stereoRender_warpTargetFraction.GetFloat();
				
				// Always scissor to the half-screen boundary, but the viewports
				// might cross that boundary if the lenses can be adjusted closer
				// together.
				glViewport( ( glConfig.nativeScreenWidth >> 1 ) - pixelDimensions,
							( glConfig.nativeScreenHeight >> 1 ) - ( pixelDimensions >> 1 ),
							pixelDimensions, pixelDimensions );
				glScissor( 0, 0, glConfig.nativeScreenWidth >> 1, glConfig.nativeScreenHeight );
				
				idVec4	color( stereoRender_warpCenterX.GetFloat(), stereoRender_warpCenterY.GetFloat(), stereoRender_warpParmZ.GetFloat(), stereoRender_warpParmW.GetFloat() );
				// don't use GL_Color(), because we don't want to clamp
				renderProgManager.SetRenderParm( RENDERPARM_COLOR, color.ToFloatPtr() );
				
				GL_SelectTexture( 0 );
				stereoRenderImages[0]->Bind();
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
				RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
				
				idVec4	color2( stereoRender_warpCenterX.GetFloat(), stereoRender_warpCenterY.GetFloat(), stereoRender_warpParmZ.GetFloat(), stereoRender_warpParmW.GetFloat() );
				// don't use GL_Color(), because we don't want to clamp
				renderProgManager.SetRenderParm( RENDERPARM_COLOR, color2.ToFloatPtr() );
				
				glViewport( ( glConfig.nativeScreenWidth >> 1 ),
							( glConfig.nativeScreenHeight >> 1 ) - ( pixelDimensions >> 1 ),
							pixelDimensions, pixelDimensions );
				glScissor( glConfig.nativeScreenWidth >> 1, 0, glConfig.nativeScreenWidth >> 1, glConfig.nativeScreenHeight );
				
				GL_SelectTexture( 0 );
				stereoRenderImages[1]->Bind();
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
				glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
				RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
				break;
			}
		// a non-warped side-by-side-uncompressed (dual input cable) is rendered
		// just like STEREO3D_SIDE_BY_SIDE_COMPRESSED, so fall through.
		case STEREO3D_SIDE_BY_SIDE_COMPRESSED:
			GL_SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			GL_SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			GL_ViewportAndScissor( 0, 0, renderSystemWidth, renderSystemHeight );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			GL_SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			GL_SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			GL_ViewportAndScissor( renderSystemWidth, 0, renderSystemWidth, renderSystemHeight );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			break;

		case STEREO3D_OPENVR:
			if (glConfig.openVREnabled)
			{
				VR_PreSwap(stereoRenderImages[0]->GetTexNum(), stereoRenderImages[1]->GetTexNum());
			}

			GL_SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			GL_SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			GL_ViewportAndScissor( 0, 0, glConfig.nativeScreenWidth/2, glConfig.nativeScreenHeight );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			GL_SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			GL_SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			GL_ViewportAndScissor( glConfig.nativeScreenWidth/2, 0, glConfig.nativeScreenWidth/2, glConfig.nativeScreenHeight );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			break;

		case STEREO3D_TOP_AND_BOTTOM_COMPRESSED:
			GL_SelectTexture( 1 );
			stereoRenderImages[0]->Bind();
			GL_SelectTexture( 0 );
			stereoRenderImages[1]->Bind();
			GL_ViewportAndScissor( 0, 0, renderSystemWidth, renderSystemHeight );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			GL_SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			GL_SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			GL_ViewportAndScissor( 0, renderSystemHeight, renderSystemWidth, renderSystemHeight );
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			break;
			
		case STEREO3D_INTERLACED:
			// every other scanline
			GL_SelectTexture( 0 );
			stereoRenderImages[0]->Bind();
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			
			GL_SelectTexture( 1 );
			stereoRenderImages[1]->Bind();
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			
			GL_ViewportAndScissor( 0, 0, renderSystemWidth, renderSystemHeight * 2 );
			renderProgManager.BindShader_StereoInterlace();
			RB_DrawElementsWithCounters( &backEnd.unitSquareSurface );
			
			GL_SelectTexture( 0 );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			
			GL_SelectTexture( 1 );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			
			break;
	}
	
	// debug tool
	RB_DrawFlickerBox();
	
	// make sure the drawing is actually started
	glFlush();
	
	// we may choose to sync to the swapbuffers before the next frame
	
	// stop rendering on this thread
	uint64 backEndFinishTime = Sys_Microseconds();
	backEnd.pc.totalMicroSec = backEndFinishTime - backEndStartTime;
}

/*
====================
RB_ExecuteBackEndCommands

This function will be called syncronously if running without
smp extensions, or asyncronously by another thread.
====================
*/
void RB_ExecuteBackEndCommands( const emptyCommand_t* cmds )
{
	// r_debugRenderToTexture
	int c_draw3d = 0;
	int c_draw2d = 0;
	int c_setBuffers = 0;
	int c_copyRenders = 0;
	
	resolutionScale.SetCurrentGPUFrameTime( commonLocal.GetRendererGPUMicroseconds() );
	
	renderLog.StartFrame();
	
	if( cmds->commandId == RC_NOP && !cmds->next )
	{
		return;
	}
	
	if( renderSystem->GetStereo3DMode() != STEREO3D_OFF )
	{
		RB_StereoRenderExecuteBackEndCommands( cmds );
		renderLog.EndFrame();
		return;
	}
	
	uint64 backEndStartTime = Sys_Microseconds();
	
	// needed for editor rendering
	GL_SetDefaultState();
	
	// If we have a stereo pixel format, this will draw to both
	// the back left and back right buffers, which will have a
	// performance penalty.
	glDrawBuffer( GL_BACK );
	
	for( ; cmds != NULL; cmds = ( const emptyCommand_t* )cmds->next )
	{
		switch( cmds->commandId )
		{
			case RC_NOP:
				break;
			case RC_DRAW_VIEW_3D:
			case RC_DRAW_VIEW_GUI:
				RB_DrawView( cmds, 0 );
				if( ( ( const drawSurfsCommand_t* )cmds )->viewDef->viewEntitys )
				{
					c_draw3d++;
				}
				else
				{
					c_draw2d++;
				}
				break;
			case RC_SET_BUFFER:
				//RB_SetBuffer( cmds );
				c_setBuffers++;
				break;
			case RC_COPY_RENDER:
				RB_CopyRender( cmds );
				c_copyRenders++;
				break;
			case RC_POST_PROCESS:
				RB_PostProcess( cmds );
				break;
			default:
				common->Error( "RB_ExecuteBackEndCommands: bad commandId" );
				break;
		}
	}
	
	RB_DrawFlickerBox();
	
	// Fix for the steam overlay not showing up while in game without Shell/Debug/Console/Menu also rendering
	glColorMask( 1, 1, 1, 1 );
	
	glFlush();
	
	// stop rendering on this thread
	uint64 backEndFinishTime = Sys_Microseconds();
	backEnd.pc.totalMicroSec = backEndFinishTime - backEndStartTime;
	
	if( r_debugRenderToTexture.GetInteger() == 1 )
	{
		common->Printf( "3d: %i, 2d: %i, SetBuf: %i, CpyRenders: %i, CpyFrameBuf: %i\n", c_draw3d, c_draw2d, c_setBuffers, c_copyRenders, backEnd.pc.c_copyFrameBuffer );
		backEnd.pc.c_copyFrameBuffer = 0;
	}
	renderLog.EndFrame();
}

extern vr::IVRSystem * hmd;
extern float g_vrScaleX;
extern float g_vrScaleY;
extern float g_vrScaleZ;
extern vr::TrackedDeviceIndex_t g_openVRLeftController;
extern vr::TrackedDeviceIndex_t g_openVRRightController;
extern idVec3 g_SeatedOrigin;
extern idMat3 g_SeatedAxis;
extern idMat3 g_SeatedAxisInverse;

bool g_vrLeftControllerWasPressed;
vr::VRControllerState_t g_vrLeftControllerState;
vr::VRControllerState_t g_vrRightControllerState;
int g_openVRLeftControllerPulseDur;
int g_openVRRightControllerPulseDur;

bool g_vrHasHeadPose;
idVec3 g_vrHeadOrigin;
idMat3 g_vrHeadAxis;
bool g_vrHadHead;
idVec3 g_vrHeadLastOrigin;
idVec3 g_vrHeadMoveDelta;

bool g_vrHasLeftControllerPose;
idVec3 g_vrLeftControllerOrigin;
idMat3 g_vrLeftControllerAxis;

bool g_vrHasRightControllerPose;
idVec3 g_vrRightControllerOrigin;
idMat3 g_vrRightControllerAxis;

bool g_poseReset;

void VR_ResetPose()
{
	g_poseReset = true;
	hmd->ResetSeatedZeroPose();
}


#define MAX_VREVENTS 256

int g_vrSysEventIndex;
int g_vrSysEventCount;
sysEvent_t g_vrSysEvents[MAX_VREVENTS];

int g_vrJoyEventCount;
struct {
	int action;
	int value;
} g_vrJoyEvents[MAX_VREVENTS];

void VR_ClearEvents()
{
	g_vrSysEventIndex = 0;
	g_vrSysEventCount = 0;
	g_vrJoyEventCount = 0;
	g_vrLeftControllerWasPressed = false;
}

void VR_SysEventQue(sysEventType_t type, int value, int value2)
{
	assert(g_vrSysEventCount < MAX_VREVENTS);
	sysEvent_t * ev = &g_vrSysEvents[g_vrSysEventCount++];

	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = 0;
	ev->evPtr = NULL;
	ev->inputDevice = 0;
}

const sysEvent_t &VR_SysEventNext()
{
	assert(g_vrSysEventIndex < MAX_VREVENTS);
	if (g_vrSysEventIndex >= g_vrSysEventCount)
	{
		sysEvent_t &ev = g_vrSysEvents[g_vrSysEventIndex];
		ev.evType = SE_NONE;
		return ev;
	}
	return g_vrSysEvents[g_vrSysEventIndex++];
}

int VR_PollJoystickInputEvents()
{
	return g_vrJoyEventCount;
}

void VR_JoyEventQue( int action, int value )
{
	assert(g_vrJoyEventCount < MAX_VREVENTS);
	g_vrJoyEvents[g_vrJoyEventCount].action = action;
	g_vrJoyEvents[g_vrJoyEventCount].value = value;
	g_vrJoyEventCount++;
}

int VR_ReturnJoystickInputEvent( const int n, int& action, int& value )
{
	if (n < 0 || n > g_vrJoyEventCount)
	{
		return 0;
	}
	action = g_vrJoyEvents[n].action;
	value = g_vrJoyEvents[n].value;
	return 1;
}

static void VR_GenButtonEvent(uint32_t button, bool left, bool pressed)
{
	switch(button)
	{
	case vr::k_EButton_ApplicationMenu:
		if (left)
		{
			VR_JoyEventQue( J_ACTION10, pressed ); // pda
			VR_SysEventQue( SE_KEY, K_JOY10, pressed ); // pda
		}
		else
		{
			VR_JoyEventQue( J_ACTION9, pressed ); // pause menu
			VR_SysEventQue( SE_KEY, K_JOY9, pressed ); // pause menu
		}
		break;
	case vr::k_EButton_Grip:
		if (left)
		{
			//VR_JoyEventQue( J_ACTION5, pressed ); //  prev weapon
			VR_JoyEventQue( J_AXIS_LEFT_TRIG, pressed? 255*128 : 0 ); // flashlight
			VR_SysEventQue( SE_KEY, K_JOY5, pressed ); // prev pda menu
		}
		else
		{
			//VR_JoyEventQue( J_ACTION6, pressed ); // next weapon
			VR_JoyEventQue( J_ACTION3, pressed ); // reload weapon
			VR_SysEventQue( SE_KEY, K_JOY6, pressed ); // next pda menu
		}
		break;
	case vr::k_EButton_SteamVR_Trigger:
		if (left)
		{
			VR_JoyEventQue( J_ACTION1, pressed ); // jump
			VR_SysEventQue( SE_KEY, K_JOY2, pressed ); // menu back
		}
		else
		{
			VR_JoyEventQue( J_AXIS_RIGHT_TRIG, pressed? 255*128 : 0 ); // fire weapon
			VR_SysEventQue( SE_KEY, K_MOUSE1, pressed ); // cursor click
		}
		break;
	case vr::k_EButton_SteamVR_Touchpad:
		if (left)
		{
			if (pressed)
			{
				g_vrLeftControllerWasPressed = true;
			}
			//VR_JoyEventQue( J_AXIS_LEFT_TRIG, pressed? 255*128 : 0 ); // flashlight
			//VR_JoyEventQue( J_ACTION7, pressed ); // run
			//VR_SysEventQue( SE_KEY, K_JOY2, pressed ); // menu back
			static keyNum_t uiLastKey;
			if (pressed)
			{
				if( g_vrLeftControllerState.rAxis[0].x < g_vrLeftControllerState.rAxis[0].y )
				{
					if( g_vrLeftControllerState.rAxis[0].x > -g_vrLeftControllerState.rAxis[0].y )
					{
						uiLastKey = K_JOY_STICK1_UP;
					}
					else
					{
						uiLastKey = K_JOY_STICK1_LEFT;
					}
				}
				else
				{
					if( g_vrLeftControllerState.rAxis[0].x > -g_vrLeftControllerState.rAxis[0].y )
					{
						uiLastKey = K_JOY_STICK1_RIGHT;
					}
					else
					{
						uiLastKey = K_JOY_STICK1_DOWN;
					}
				}
				VR_SysEventQue( SE_KEY, uiLastKey, 1 );
			}
			else
			{
				VR_SysEventQue( SE_KEY, uiLastKey, 0 );
			}
		}
		else
		{
			//VR_JoyEventQue( J_ACTION3, pressed ); // reload weapon
			if( vr_turning.GetInteger() == 0 )
			{
				if (g_vrRightControllerState.rAxis[0].x < -0.4f)
				{
					VR_JoyEventQue( J_ACTION5, pressed ); //  prev weapon
				}
				else if (g_vrRightControllerState.rAxis[0].x > 0.4f)
				{
					VR_JoyEventQue( J_ACTION6, pressed ); //  next weapon
				}
				else
				{
					VR_JoyEventQue( J_ACTION3, pressed ); // reload weapon
				}
			}
			else
			{
				if (g_vrRightControllerState.rAxis[0].y < -0.5f)
				{
					VR_JoyEventQue( J_ACTION5, pressed ); //  prev weapon
				}
				else if (g_vrRightControllerState.rAxis[0].y > 0.5f)
				{
					VR_JoyEventQue( J_ACTION6, pressed ); //  next weapon
				}
			}
			VR_SysEventQue( SE_KEY, K_JOY1, pressed ); // menu select
		}
		break;
	default:
		break;
	}
}

static void VR_GenJoyAxisEvents()
{
	if (g_openVRLeftController != vr::k_unTrackedDeviceIndexInvalid)
	{
		vr::VRControllerState_t &state = g_vrLeftControllerState;
		hmd->GetControllerState(g_openVRLeftController, &state);
	}
	if (g_openVRRightController != vr::k_unTrackedDeviceIndexInvalid)
	{
		vr::VRControllerState_t &state = g_vrRightControllerState;
		hmd->GetControllerState(g_openVRRightController, &state);
	}
}

static void VR_GenMouseEvents()
{
	// virtual head tracking mouse for shell UI
	idVec3 shellOrigin;
	idMat3 shellAxis;
	if (g_vrHadHead && tr.guiModel->GetVRShell(shellOrigin, shellAxis))
	{
		const float virtualWidth = renderSystem->GetVirtualWidth();
		const float virtualHeight = renderSystem->GetVirtualHeight();
		float guiHeight = 12*5.3f;
		float guiScale = guiHeight / virtualHeight;
		float guiWidth = virtualWidth * guiScale;
		float guiForward = guiHeight + 12.f;
		idVec3 upperLeft = shellOrigin
			+ shellAxis[0] * guiForward
			+ shellAxis[1] * 0.5f * guiWidth
			+ shellAxis[2] * 0.5f * guiHeight;
		idMat3 invShellAxis = shellAxis.Inverse();
		idVec3 rayStart = (g_vrHeadOrigin - upperLeft) * invShellAxis;
		idVec3 rayDir = g_vrHeadAxis[0] * invShellAxis;
		if (rayDir.x != 0)
		{
			static int oldX, oldY;
			float wx = rayStart.y - rayStart.x * rayDir.y / rayDir.x;
			float wy = rayStart.z - rayStart.x * rayDir.z / rayDir.x;
			int x = -wx * glConfig.nativeScreenWidth / guiWidth;
			int y = -wy * glConfig.nativeScreenHeight / guiHeight;
			if (x >= 0 && x < glConfig.nativeScreenWidth &&
				y >= 0 && y < glConfig.nativeScreenHeight &&
				(x!= oldX || y != oldY))
			{
				oldX = x;
				oldY = y;
				VR_SysEventQue( SE_MOUSE_ABSOLUTE, x, y );
			}
		}
	}
}

void VR_ConvertMatrix(const vr::HmdMatrix34_t &poseMat, idVec3 &origin, idMat3 &axis)
{
	origin.Set(
		g_vrScaleX * poseMat.m[2][3],
		g_vrScaleY * poseMat.m[0][3],
		g_vrScaleZ * poseMat.m[1][3] );
	axis[0].Set(poseMat.m[2][2], poseMat.m[0][2], -poseMat.m[1][2]);
	axis[1].Set(poseMat.m[2][0], poseMat.m[0][0], -poseMat.m[1][0]);
	axis[2].Set(-poseMat.m[2][1], -poseMat.m[0][1], poseMat.m[1][1]);
}

bool VR_ConvertPose(const vr::TrackedDevicePose_t &pose, idVec3 &origin, idMat3 &axis)
{
	if (!pose.bPoseIsValid)
	{
		return false;
	}

	VR_ConvertMatrix(pose.mDeviceToAbsoluteTracking, origin, axis);

	return true;
}

void VR_UpdateResolution()
{
	vr_resolutionScale.ClearModified();
	float scale = vr_resolutionScale.GetFloat();
	uint32_t width, height;
	hmd->GetRecommendedRenderTargetSize( &width, &height );
	width = width * scale;
	height = height * scale;
	if (width < 540) width = 640;
	else if (width > 8000) width = 8000;
	if (height < 540) height = 480;
	else if (height > 8000) height = 8000;
	glConfig.openVRWidth = width;
	glConfig.openVRHeight = height;
}

void VR_UpdateScaling()
{
	const float m2i = 1 / 0.0254f; // meters to inches
	const float cm2i = 1 / 2.54f; // centimeters to inches
	float ratio = 76.5f / (vr_playerHeightCM.GetFloat() * cm2i); // converts player height to character height
	glConfig.openVRScale = m2i * ratio;
	glConfig.openVRHalfIPD = glConfig.openVRUnscaledHalfIPD * glConfig.openVRScale;
	glConfig.openVREyeForward = glConfig.openVRUnscaledEyeForward * glConfig.openVRScale;
	g_vrScaleX = -glConfig.openVRScale;
	g_vrScaleY = -glConfig.openVRScale;
	g_vrScaleZ = glConfig.openVRScale;
}

void VR_PreSwap(GLuint left, GLuint right)
{
	GL_ViewportAndScissor(0, 0, glConfig.openVRWidth, glConfig.openVRHeight);
	vr::Texture_t leftEyeTexture = {(void*)left, vr::API_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
	vr::Texture_t rightEyeTexture = {(void*)right, vr::API_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
}

void VR_PostSwap()
{
	//vr::VRCompositor()->PostPresentHandoff();

	vr::TrackedDevicePose_t rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	vr::VRCompositor()->WaitGetPoses(rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	bool hadLeft = g_openVRLeftController != vr::k_unTrackedDeviceIndexInvalid;
	bool hadRight = g_openVRRightController != vr::k_unTrackedDeviceIndexInvalid;

	g_openVRLeftController = hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
	g_openVRRightController = hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

	if( hadLeft && g_openVRLeftController == vr::k_unTrackedDeviceIndexInvalid )
	{
		common->Printf("left controller lost\n");
	}
	if( hadRight && g_openVRRightController == vr::k_unTrackedDeviceIndexInvalid )
	{
		common->Printf("right controller lost\n");
	}

	if (g_openVRLeftController != vr::k_unTrackedDeviceIndexInvalid
		|| g_openVRRightController != vr::k_unTrackedDeviceIndexInvalid)
	{
		if (glConfig.openVRSeated)
		{
			glConfig.openVRSeated = false;
		}
	}
	else
	{
		if (!glConfig.openVRSeated)
		{
			glConfig.openVRSeated = true;
		}
	}

	if (vr_playerHeightCM.IsModified())
	{
		vr_playerHeightCM.ClearModified();
		VR_UpdateScaling();
	}

	if( vr_seated.IsModified() )
	{
		vr_seated.ClearModified();
		tr.guiModel->UpdateVRShell();
	}

	vr::TrackedDevicePose_t &hmdPose = rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd];
	g_vrHasHeadPose = hmdPose.bPoseIsValid;
	if (hmdPose.bPoseIsValid)
	{
		VR_ConvertPose( hmdPose, g_vrHeadOrigin, g_vrHeadAxis );
		g_vrHeadOrigin += glConfig.openVREyeForward * g_vrHeadAxis[2];

		if (g_vrHadHead)
		{
			g_vrHeadMoveDelta = g_vrHeadOrigin - g_vrHeadLastOrigin;
			g_vrHeadLastOrigin = g_vrHeadOrigin;
		}
		else
		{
			g_vrHadHead = true;
			g_vrHeadMoveDelta.Zero();
		}
	}
	else
	{
		g_vrHadHead = false;
	}

	if( vr_forceGamepad.GetBool() )
	{
		g_vrHasLeftControllerPose = false;
		g_vrHasRightControllerPose = false;
	}
	else
	{
		if( g_openVRLeftController != vr::k_unTrackedDeviceIndexInvalid )
		{
			if(g_openVRLeftControllerPulseDur > 500)
			{
				hmd->TriggerHapticPulse(g_openVRLeftController, 0, g_openVRLeftControllerPulseDur);
			}
			g_openVRLeftControllerPulseDur = 0;

			static bool hadLeftPose;
			vr::TrackedDevicePose_t &handPose = rTrackedDevicePose[g_openVRLeftController];
			if( handPose.bPoseIsValid )
			{
				g_vrHasLeftControllerPose = true;
				VR_ConvertPose( handPose, g_vrLeftControllerOrigin, g_vrLeftControllerAxis );
				hadLeftPose = true;
			}
			else if (hadLeftPose)
			{
				hadLeftPose = false;
				common->Printf("left controller had no pose\n");
			}
		}

		if( g_openVRRightController != vr::k_unTrackedDeviceIndexInvalid )
		{
			if(g_openVRRightControllerPulseDur > 500)
			{
				hmd->TriggerHapticPulse(g_openVRRightController, 0, g_openVRRightControllerPulseDur);
			}
			g_openVRRightControllerPulseDur = 0;

			static bool hadRightPose;
			vr::TrackedDevicePose_t &handPose = rTrackedDevicePose[g_openVRRightController];
			if( handPose.bPoseIsValid )
			{
				g_vrHasRightControllerPose = true;
				VR_ConvertPose( handPose, g_vrRightControllerOrigin, g_vrRightControllerAxis );
				hadRightPose = true;
			}
			else if (hadRightPose)
			{
				hadRightPose = false;
				common->Printf("right controller had no pose\n");
			}
		}
	}

	VR_ClearEvents();

	VR_GenJoyAxisEvents();

	vr::VREvent_t e;
	while( hmd->PollNextEvent( &e, sizeof( e ) ) )
	{
		//vr::ETrackedControllerRole role;

		switch (e.eventType)
		{
		/*case vr::VREvent_TrackedDeviceActivated:
			role = hmd->GetControllerRoleForTrackedDeviceIndex(e.trackedDeviceIndex);
			switch(role)
			{
			case vr::TrackedControllerRole_LeftHand:
				g_openVRLeftController = e.trackedDeviceIndex;
				break;
			case vr::TrackedControllerRole_RightHand:
				g_openVRRightController = e.trackedDeviceIndex;
				break;
			}
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			if (e.trackedDeviceIndex == g_openVRLeftController)
			{
				g_openVRLeftController = vr::k_unTrackedDeviceIndexInvalid;
			}
			else if (e.trackedDeviceIndex == g_openVRRightController)
			{
				g_openVRRightController = vr::k_unTrackedDeviceIndexInvalid;
			}
			break;*/
		case vr::VREvent_ButtonPress:
			if (e.trackedDeviceIndex == g_openVRLeftController || e.trackedDeviceIndex == g_openVRRightController)
			{
				VR_GenButtonEvent(e.data.controller.button, e.trackedDeviceIndex == g_openVRLeftController, true);
			}
			break;
		case vr::VREvent_ButtonUnpress:
			if (e.trackedDeviceIndex == g_openVRLeftController || e.trackedDeviceIndex == g_openVRRightController)
			{
				VR_GenButtonEvent(e.data.controller.button, e.trackedDeviceIndex == g_openVRLeftController, false);
			}
			break;
		}
	}

	if( !glConfig.openVRSeated )
	{
		VR_GenMouseEvents();
	}

	if (g_poseReset)
	{
		g_poseReset = false;
		VR_ConvertMatrix(hmd->GetSeatedZeroPoseToStandingAbsoluteTrackingPose(), g_SeatedOrigin, g_SeatedAxis);
		g_SeatedAxisInverse = g_SeatedAxis.Inverse();
		tr.guiModel->UpdateVRShell();
	}
}

bool VR_CalculateView(idVec3 &origin, idMat3 &axis, const idVec3 &eyeOffset, bool overridePitch)
{
	if (!g_vrHasHeadPose)
	{
		return false;
	}

	if (overridePitch)
	{
		float pitch = idMath::M_RAD2DEG * asin(axis[0][2]);
		idAngles angles(pitch, 0, 0);
		axis = angles.ToMat3() * axis;
	}

	if (!vr_seated.GetBool())
	{
		origin.z -= eyeOffset.z;
		// ignore x and y
		origin += axis[2] * g_vrHeadOrigin.z;
	}
	else
	{
		origin += axis * g_vrHeadOrigin;
	}

	axis = g_vrHeadAxis * axis;

	return true;
}

bool VR_GetHead(idVec3 &origin, idMat3 &axis)
{
	if (!g_vrHasHeadPose)
	{
		return false;
	}

	origin = g_vrHeadOrigin;
	axis = g_vrHeadAxis;

	return true;
}

// returns left controller position relative to the head
bool VR_GetLeftController(idVec3 &origin, idMat3 &axis)
{
	if (!g_vrHasLeftControllerPose || !g_vrHasHeadPose)
	{
		return false;
	}

	origin = g_vrLeftControllerOrigin;
	axis = g_vrLeftControllerAxis;

	return true;
}

// returns right controller position relative to the head
bool VR_GetRightController(idVec3 &origin, idMat3 &axis)
{
	if (!g_vrHasRightControllerPose || !g_vrHasHeadPose)
	{
		return false;
	}

	origin = g_vrRightControllerOrigin;
	axis = g_vrRightControllerAxis;

	return true;
}

void VR_MoveDelta(idVec3 &delta, float &height)
{
	if (!g_vrHasHeadPose)
	{
		height = 0.f;
		delta.Set(0,0,0);
		return;
	}

	height = g_vrHeadOrigin.z;

	delta.x = g_vrHeadMoveDelta.x;
	delta.y = g_vrHeadMoveDelta.y;
	delta.z = 0.f;

	g_vrHeadMoveDelta.Zero();
}

void VR_HapticPulse(int leftDuration, int rightDuration)
{
	if( leftDuration > g_openVRLeftControllerPulseDur )
	{
		g_openVRLeftControllerPulseDur = leftDuration;
	}
	if( rightDuration > g_openVRRightControllerPulseDur )
	{
		g_openVRRightControllerPulseDur = rightDuration;
	}
}

bool VR_GetLeftControllerAxis(idVec2 &axis)
{
	if( g_openVRLeftController == vr::k_unTrackedDeviceIndexInvalid )
	{
		return false;
	}
	uint64_t mask = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
	if( !( g_vrLeftControllerState.ulButtonTouched & mask ) )
	{
		return false;
	}
	axis.x = g_vrLeftControllerState.rAxis[0].x;
	axis.y = g_vrLeftControllerState.rAxis[0].y;
	return true;
}

bool VR_GetRightControllerAxis(idVec2 &axis)
{
	if( g_openVRRightController == vr::k_unTrackedDeviceIndexInvalid )
	{
		return false;
	}
	uint64_t mask = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
	if( !( g_vrRightControllerState.ulButtonTouched & mask ) )
	{
		return false;
	}
	axis.x = g_vrRightControllerState.rAxis[0].x;
	axis.y = g_vrRightControllerState.rAxis[0].y;
	return true;
}

bool VR_LeftControllerWasPressed()
{
	return g_vrLeftControllerWasPressed;
}

bool VR_LeftControllerIsPressed()
{
	uint64_t mask = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
	return ( g_vrLeftControllerState.ulButtonPressed & mask ) != 0;
}

const idVec3 &VR_GetSeatedOrigin()
{
	return g_SeatedOrigin;
}

const idMat3 &VR_GetSeatedAxis()
{
	return g_SeatedAxis;
}

const idMat3 &VR_GetSeatedAxisInverse()
{
	return g_SeatedAxisInverse;
}

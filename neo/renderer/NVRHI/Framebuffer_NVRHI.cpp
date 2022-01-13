/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014-2020 Robert Beckebans

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

#include "../RenderCommon.h"
#include "../Framebuffer.h"

#if !defined(USE_VULKAN)

static void R_ListFramebuffers_f( const idCmdArgs& args )
{
	if( !glConfig.framebufferObjectAvailable )
	{
		common->Printf( "GL_EXT_framebuffer_object is not available.\n" );
		return;
	}
}

Framebuffer::Framebuffer( const char* name, int w, int h )
	: fboName(name)
	, frameBuffer(0)
	, colorFormat(0)
	, depthBuffer(0)
	, depthFormat(0)
	, stencilFormat(0)
	, stencilBuffer(0)
	, width(w)
	, height(h)
	, msaaSamples(false)
{
	memset( colorBuffers, 0, sizeof( colorBuffers ) );
	framebuffers.Append( this );
}

Framebuffer::~Framebuffer()
{
}

void Framebuffer::Init()
{
	cmdSystem->AddCommand( "listFramebuffers", R_ListFramebuffers_f, CMD_FL_RENDERER, "lists framebuffers" );
}

void Framebuffer::CheckFramebuffers()
{
	int screenWidth = renderSystem->GetWidth();
	int screenHeight = renderSystem->GetHeight();
}

void Framebuffer::Shutdown()
{
	framebuffers.DeleteContents( true );
}

void Framebuffer::Bind()
{
	RENDERLOG_PRINTF( "Framebuffer::Bind( %s )\n", fboName.c_str() );
}

bool Framebuffer::IsBound()
{
	return false;
}

void Framebuffer::Unbind()
{
	RENDERLOG_PRINTF( "Framebuffer::Unbind()\n" );
}

bool Framebuffer::IsDefaultFramebufferActive()
{
	return false;
}

Framebuffer* Framebuffer::GetActiveFramebuffer()
{
	return nullptr;
}

void Framebuffer::AddColorBuffer( int format, int index, int multiSamples )
{
}

void Framebuffer::AddDepthBuffer( int format, int multiSamples )
{
}

void Framebuffer::AddStencilBuffer( int format, int multiSamples )
{
}

void Framebuffer::AttachImage2D( int target, const idImage* image, int index, int mipmapLod )
{
}

void Framebuffer::AttachImageDepth( int target, const idImage* image )
{
}

void Framebuffer::AttachImageDepthLayer( const idImage* image, int layer )
{
}

void Framebuffer::Check()
{
}

#endif // #if !defined(USE_VULKAN)
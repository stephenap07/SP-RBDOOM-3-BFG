/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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

#include "RmlUserInterfaceLocal.h"
#include "RmlFileSystem.h"

#include "renderer/GLState.h"
#include "renderer/GuiModel.h"
#include "renderer/Image.h"


RmlUserInterfaceManagerLocal rmlManagerLocal;
RmlUserInterfaceManager* rmlManager = &rmlManagerLocal;

idDeviceContext* rmlDc;

extern idCVar sys_lang;

/*
===============
RmlUserInterfaceLocal

UI to represent a context. Handles events and contains rml file loading.
===============
*/

RmlUserInterfaceLocal::RmlUserInterfaceLocal()
	: _context( nullptr )
	, _name()
	, _timeStamp( 0 )
	, _cursorX( 0 )
	, _cursorY( 0 )
	, _cursorEnabled( true )
	, _isActive( false )
	, _isPausingGame( false )
	, _inhibitsControl( false )
	, _refs( 1 )
{
}

RmlUserInterfaceLocal::~RmlUserInterfaceLocal()
{
}

bool RmlUserInterfaceLocal::Init( const char* name )
{
	_context = Rml::CreateContext( name, Rml::Vector2i( renderSystem->GetWidth(), renderSystem->GetHeight() ) );
	_context->EnableMouseCursor( true );
	return _context != nullptr;
}

const char* RmlUserInterfaceLocal::HandleEvent( const sysEvent_t* event, int time, bool* updateVisuals )
{
	int keyModState = idRmlSystem::GetKeyModifier();

	if( event->IsMouseEvent() )
	{
		_cursorX += event->evValue;
		_cursorY += event->evValue2;

		if( _cursorX < 0 )
		{
			_cursorX = 0;
		}

		if( _cursorY < 0 )
		{
			_cursorY = 0;
		}

		if( ( _cursorX + 1 ) > _context->GetDimensions().x )
		{
			_cursorX = _context->GetDimensions().x - 1;
		}

		if( ( _cursorY + 1 ) > _context->GetDimensions().y )
		{
			_cursorY = _context->GetDimensions().y - 1;
		}

		_context->ProcessMouseMove( _cursorX, _cursorY, keyModState );
	}
	else if( event->IsMouseAbsoluteEvent() )
	{
		_cursorX = event->evValue;
		_cursorY = event->evValue2;

		const float pixelAspect = renderSystem->GetPixelAspect();
		const float sysWidth = renderSystem->GetWidth() * ( pixelAspect > 1.0f ? pixelAspect : 1.0f );
		const float sysHeight = renderSystem->GetHeight() / ( pixelAspect < 1.0f ? pixelAspect : 1.0f );
		float scale = 1.0f * sysHeight / ( float )_context->GetDimensions().y;
		float invScale = 1.0f / scale;
		float tx = 0.5f * ( sysWidth - ( _context->GetDimensions().x * scale ) );
		float ty = 0.5f * ( sysHeight - ( _context->GetDimensions().y * scale ) );

		_cursorX = idMath::Ftoi( ( static_cast<float>( event->evValue ) - tx ) * invScale );
		_cursorY = idMath::Ftoi( ( static_cast<float>( event->evValue2 ) - ty ) * invScale );

		if( _cursorX < 0 )
		{
			_cursorX = 0;
		}

		if( _cursorY < 0 )
		{
			_cursorY = 0;
		}

		if( ( _cursorX + 1 ) > _context->GetDimensions().x )
		{
			_cursorX = _context->GetDimensions().x - 1;
		}

		if( ( _cursorY + 1 ) > _context->GetDimensions().y )
		{
			_cursorY = _context->GetDimensions().y - 1;
		}

		_context->ProcessMouseMove( _cursorX, _cursorY, keyModState );
	}

	if( event->evValue >= K_MOUSE1 && event->evValue <= K_MOUSE16 )
	{
		int mouseButton = 0;

		switch( event->evValue )
		{
			case K_MOUSE1:
				mouseButton = 0;
				break;
			case K_MOUSE2:
				mouseButton = 1;
				break;
			case K_MOUSE3:
				mouseButton = 2;
				break;
			case K_MOUSE4:
				mouseButton = 3;
				break;
			default:
				mouseButton = 0;
		}

		if( event->evValue2 )
		{
			_context->ProcessMouseButtonDown( mouseButton, keyModState );
		}
		else
		{
			_context->ProcessMouseButtonUp( mouseButton, keyModState );
		}
	}

	if( event->evValue == K_MWHEELDOWN || event->evValue == K_MWHEELUP )
	{
		float mouseWheel = ( event->evValue == K_MWHEELDOWN ) ? 1.0f : -1.0f;
		_context->ProcessMouseWheel( mouseWheel, keyModState );
	}

	if( event->IsCharEvent() )
	{
		Rml::Input::KeyIdentifier key = idRmlSystem::TranslateKey( event->evValue );

		_context->ProcessKeyDown( key, keyModState );
	}

	return nullptr;
}

void RmlUserInterfaceLocal::HandleNamedEvent( const char* eventName )
{
}

void RmlUserInterfaceLocal::Redraw( int time, bool hud )
{
	if( rmlManager->InLevelLoad() )
	{
		return;
	}

	_context->SetDimensions( Rml::Vector2i( renderSystem->GetWidth(), renderSystem->GetHeight() ) );
	_context->Update();
	( ( idRmlRender* )Rml::GetRenderInterface() )->PreRender();
	_context->Render();
	( ( idRmlRender* )Rml::GetRenderInterface() )->PostRender();
	DrawCursor();
}

RmlUi* RmlUserInterfaceLocal::AddUi( RmlUi* ui )
{
	_rmlUi.Append( ui );
	return ui;
}

void RmlUserInterfaceLocal::Reload()
{
}

const char* RmlUserInterfaceLocal::Activate( bool activate, int time )
{
	_isActive = activate;
	return nullptr;
}

size_t RmlUserInterfaceLocal::Size()
{
	return size_t();
}

void RmlUserInterfaceLocal::DrawCursor()
{
	if( !_cursorEnabled )
	{
		return;
	}

	const idVec2 scaleToVirtual( ( float )renderSystem->GetVirtualWidth() / renderSystem->GetWidth(),
								 ( float )renderSystem->GetVirtualHeight() / renderSystem->GetHeight() );
	idVec2 bounds;
	bounds.x = _context->GetDimensions().x * scaleToVirtual.x;
	bounds.y = _context->GetDimensions().y * scaleToVirtual.y;
	float x = _cursorX * scaleToVirtual.x;
	float y = _cursorY * scaleToVirtual.y;
	rmlDc->DrawCursor( &x, &y, 36.0f * scaleToVirtual.x, bounds );
}

static void Cmd_ReloadRml( const idCmdArgs& args )
{
	rmlManagerLocal.Reload( true );
}

/*
===============
RmlUserInterfaceManagerLocal

Manages a group of rml UI contexts and manages the lifetime of these objects through level loading.
===============
*/

void RmlUserInterfaceManagerLocal::Init()
{
	_rmlRender.Init();

	Rml::SetRenderInterface( &_rmlRender );
	Rml::SetSystemInterface( &_rmlSystem );
	Rml::SetFileInterface( &_rmlFileSystem );

	if( !Rml::Initialise() )
	{
		common->Error( "Failed to initialize RML UI system" );
	}

	// Make this font face stuff configurable. Or put it in one of the base classes.
	struct FontFace
	{
		Rml::String filename;
		bool fallbackFace;
	};

	FontFace fontFaces[] =
	{
		{ "LatoLatin-Regular.ttf",    false },
		{ "LatoLatin-Italic.ttf",     false },
		{ "LatoLatin-Bold.ttf",       false },
		{ "LatoLatin-BoldItalic.ttf", false },
		{ "NotoEmoji-Regular.ttf",    true  },
	};

	for( const FontFace& face : fontFaces )
	{
		Rml::LoadFontFace( Rml::String( "newfonts/" ) + face.filename, face.fallbackFace );
	}

	_dc.Init();

	rmlDc = &_dc;

	cmdSystem->AddCommand( "reloadRml", Cmd_ReloadRml, CMD_FL_SYSTEM, "reload rml guis" );

	_inLevelLoad = false;
}

void RmlUserInterfaceManagerLocal::Shutdown()
{
	int c = _guis.Num();
	for( int i = 0; i < c; i++ )
	{
		delete _guis[i];
	}

	_guis.Clear();
	Rml::Shutdown();
}

RmlUserInterface* RmlUserInterfaceManagerLocal::Find( const char* name, bool autoload )
{
	int c = _guis.Num();
	for( int i = 0; i < c; i++ )
	{
		if( !idStr::Icmp( _guis[i]->GetName(), name ) )
		{
			return _guis[i];
		}
	}

	RmlUserInterfaceLocal* ui = new RmlUserInterfaceLocal();

	_guis.Append( ui );

	if( autoload )
	{
		ui->Init( name );
	}

	return ui;
}

void RmlUserInterfaceManagerLocal::BeginLevelLoad()
{
	_inLevelLoad = true;
}

void RmlUserInterfaceManagerLocal::EndLevelLoad( const char* mapName )
{
	_inLevelLoad = false;
}

void RmlUserInterfaceManagerLocal::Reload( bool all )
{
	for( int i = 0; i < _guis.Num(); i++ )
	{
		_guis[i]->Reload();
	}
}

void RmlUserInterfaceManagerLocal::PostRender()
{
	for( int i = 0; i < _materialsToReload.Num(); i++ )
	{
		_materialsToReload[i].image->GenerateImage(
			_materialsToReload[i].data,
			_materialsToReload[i].dimensions.x,
			_materialsToReload[i].dimensions.y,
			textureFilter_t::TF_NEAREST,
			textureRepeat_t::TR_CLAMP,
			textureUsage_t::TD_LOOKUP_TABLE_RGBA );
	}

	_materialsToReload.Clear();
}

void RmlUserInterfaceManagerLocal::AddMaterialToReload( const idMaterial* material, idImage* image, idVec2 dimensions, const byte* data )
{
	_materialsToReload.Append( RmlMaterial() );
	RmlMaterial& mat = _materialsToReload[_materialsToReload.Num() - 1];
	mat.image = image;
	mat.material = material;
	mat.data = data;
	mat.dimensions = dimensions;
}
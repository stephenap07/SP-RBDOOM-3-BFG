/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2021 Stephen Pridham

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

#include "RmlShell.h"

#include "EventHandlers.h"

#include "../Game_local.h"

#include "rmlui/RmlUserInterfaceLocal.h"
#include "rmlui/GlobalRmlEventListener.h"
#include "rmlui/RmlEventHandler.h"

#include "RmlUi/Core/EventListener.h"
#include "RmlUi/Core/EventListenerInstancer.h"
#include "RmlUi/Core/TransformPrimitive.h"

// For some reason, rml relies on these container types. It uses value_type.

class UI_Shell;

static EventHandlerOptions* eventHandlerOptions = nullptr;
static RmlGameEventHandler* baseEventHandler = nullptr;

// UI Code

UI_Shell::UI_Shell( )
	: ui( nullptr )
	, soundWorld( nullptr )
	, nextState( ShellState::WAITING )
	, state( ShellState::WAITING )
	, activeScreen( ShellScreen::START )
	, nextScreen( ShellScreen::START )
	, gameComplete( false )
	, inGame( false )
	, isInitialized( false )
{
	if( !eventHandlerOptions )
	{
		eventHandlerOptions = new EventHandlerOptions( this );
	}

	if( !baseEventHandler )
	{
		baseEventHandler = new RmlGameEventHandler( this );
	}
}

UI_Shell::~UI_Shell()
{
	delete eventHandlerOptions;
	delete baseEventHandler;
}

bool UI_Shell::Init( const char* filename,  idSoundWorld* sw )
{
	ui = rmlManager->Find( "shell", true );

	if( !ui )
	{
		return false;
	}

	isInitialized = true;

	soundWorld = sw;

	SetupDataBinding( );

	ui->SetInhibitsControl( true );
	ui->SetIsPausingGame( false );

	// Load up all the documents.
	ui->LoadDocument( "guis/rml/shell/options.rml", eventHandlerOptions );
	ui->LoadDocument( "guis/rml/shell/startmenu.rml", baseEventHandler );
	ui->LoadDocument( "guis/rml/shell/loading.rml", baseEventHandler );
	ui->LoadDocument( "guis/rml/shell/test.rml", baseEventHandler );
	ui->LoadDocument( "guis/rml/shell/pause.rml", baseEventHandler );
	ui->LoadDocument( "guis/rml/shell/game.rml", baseEventHandler );

	screenToName.AssureSize( ( int )ShellScreen::TOTAL );
	screenToName[( int )ShellScreen::START] = "guis/rml/shell/startmenu.rml";
	screenToName[( int )ShellScreen::OPTIONS] = "guis/rml/shell/options.rml";
	screenToName[( int )ShellScreen::LOADING] = "guis/rml/shell/loading.rml";
	screenToName[( int )ShellScreen::GAME] = "guis/rml/shell/game.rml";
	screenToName[( int )ShellScreen::TEST] = "guis/rml/shell/test.rml";
	screenToName[( int )ShellScreen::PAUSE] = "guis/rml/shell/pause.rml";

	// Preload all the materials.
	rmlManager->Preload( "" );

	return true;
}

struct ShellOptions
{
	std::vector<vidMode_t> vidModes;
	int windowMode;

	void Init( )
	{
		vidModes.clear( );
		idList<vidMode_t> tempModeList;
		R_GetModeListForDisplay( 0, tempModeList );
		for( int i = 0; i < tempModeList.Num( ); i++ )
		{
			vidModes.push_back( tempModeList[i] );
		}

		windowMode = r_fullscreen.GetInteger( );
	}

} shellOptions;

void UI_Shell::Update( )
{
	HandleStateChange( );

	HandleScreenChange( );

	ui->Redraw( Sys_Milliseconds( ) / 1000.0f );
}

void UI_Shell::HandleStateChange( )
{
	// State Machine
	if( nextState != state )
	{
		if( nextState == ShellState::START )
		{
			nextScreen = ShellScreen::START;

			ShowScreen( "startmenu" );

			state = nextState;
		}
		else if( nextState == ShellState::GAME )
		{
			if( state == ShellState::LOADING )
			{
				HideScreen( "loading" );
			}

			if( gameComplete )
			{
				state = ShellState::CREDITS;
			}
			else
			{
				ShowScreen( "game" );
				state = nextState;
			}
		}
		else if( nextState == ShellState::LOADING )
		{
			ShowScreen( "loading" );
			state = nextState;
		}
	}
}


void UI_Shell::HandleScreenChange( )
{
	if( activeScreen != nextScreen )
	{
		activeScreen = nextScreen;
	}

	if( !nextScreenName.IsEmpty( ) )
	{
		ShowScreen( nextScreenName.c_str( ) );
		nextScreenName.Clear( );
	}
}

void UI_Shell::SetState( ShellState _nextState )
{
	nextState = _nextState;
}

void UI_Shell::SetupDataBinding( )
{
	if( ui->IsDocumentOpen( "guis/rml/shell/options.rml" ) )
	{
		// Already loaded the document. Don't set up data binding.
		return;
	}

	shellOptions.Init( );

	Rml::DataModelConstructor constructor = ui->Context( )->CreateDataModel( "options" );

	if( !constructor )
	{
		return;
	}

	if( auto vidModeHandle = constructor.RegisterStruct<vidMode_t>( ) )
	{
		vidModeHandle.RegisterMember( "width", &vidMode_t::width );
		vidModeHandle.RegisterMember( "height", &vidMode_t::height );
		vidModeHandle.RegisterMember( "displayHz", &vidMode_t::displayHz );
	}

	constructor.RegisterArray<std::vector<vidMode_t>>( );
	constructor.Bind( "vidModes", &shellOptions.vidModes );

	vidModeModel = constructor.GetModelHandle( );
}

void UI_Shell::ActivateMenu( bool show )
{
	if( show && ui && ui->IsActive( ) )
	{
		return;
	}
	else if( !show && ui && !ui->IsActive( ) )
	{
		return;
	}

	if( inGame )
	{
		idPlayer* player = gameLocal.GetLocalPlayer( );
		if( player )
		{
			if( !show )
			{
				if( player->IsDead( ) && !common->IsMultiplayer( ) )
				{
					return;
				}
			}
		}
	}

	if( ui )
	{
		ui->Activate( show );
	}

	if( show )
	{
		if( !inGame )
		{
			//PlaySound( GUI_SOUND_MUSIC );
		}
	}
	else
	{
		nextScreen = ShellScreen::START;
		activeScreen = ShellScreen::START;
		nextState = ShellState::START;
		state = ShellState::START;
		common->Dialog( ).ClearDialog( GDM_LEAVE_LOBBY_RET_NEW_PARTY );
	}
}

void UI_Shell::SetNextScreen( ShellScreen _nextScreen )
{
	nextScreen = _nextScreen;
}

void UI_Shell::SetNextScreen( const char* _nextScreen )
{
	nextScreenName = _nextScreen;
}

void UI_Shell::ShowScreen( const char* _screen )
{
	auto doc = ui->LoadDocument( va( "guis/rml/shell/%s.rml", _screen ) );
	if( doc )
	{
		doc->Show( );
	}
}

void UI_Shell::HideScreen( const char* _screen )
{
	auto doc = ui->LoadDocument( va( "guis/rml/shell/%s.rml", _screen ) );
	if( doc )
	{
		doc->Hide( );
	}
}

void UI_Shell::UpdateSavedGames( )
{
}

bool UI_Shell::IsPausingGame( )
{
	return ui->IsPausingGame( );
}

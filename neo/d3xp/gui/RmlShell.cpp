/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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

#include "rmlui/RmlUserInterfaceLocal.h"

#include "RmlShell.h"

#include "../Game_local.h"

class EventHandler
{
public:
	virtual ~EventHandler() = default;
	virtual void ProcessEvent( Rml::Event& event, const Rml::String& value ) = 0;
};


MyEventListener::MyEventListener( UI_Shell* shell, const Rml::String& value, Rml::Element* element )
	: _shell( shell )
	, value( value )
	, element( element )
{
}

void MyEventListener::ProcessEvent( Rml::Event& event )
{
	using namespace Rml;

	idLexer src;
	idToken token;
	src.LoadMemory( value.c_str(), value.length(), "rmlCommands" );

	while( true )
	{
		if( !src.ReadToken( &token ) )
		{
			break;
		}

		if( !token.Icmp( ";" ) )
		{
			continue;
		}

		if( !token.Icmp( "exit" ) )
		{
			common->Quit();
		}

		if( !token.Icmp( "goto" ) )
		{
			src.ReadToken( &token );
			_shell->SetInhibitsControl( false );
			_shell->SetIsPausingGame( false );
			_shell->SetCursorEnabled( false );
			_shell->SetNextScreen( token.c_str() );
			event.GetTargetElement()->GetOwnerDocument()->Close();
		}

		if( !token.Icmp( "load" ) )
		{
			src.ReadToken( &token );
			_shell->SetInhibitsControl( false );
			_shell->SetIsPausingGame( false );
			_shell->SetCursorEnabled( false );
			_shell->SetNextScreen( token.c_str() );
		}

		if( !token.Icmp( "game" ) )
		{
			_shell->SetInhibitsControl( false );
			_shell->SetIsPausingGame( false );
			_shell->SetCursorEnabled( false );
			cmdSystem->AppendCommandText( "map test4\n" );
		}
	}
}

void MyEventListener::OnDetach( Rml::Element* /*element*/ )
{
	delete this;
}

// UI Code

UI_Shell::UI_Shell()
	: _eventListenerInstancer( this )
	, _ui( nullptr )
	, _isActive( false )
	, _isPausingGame( false )
	, _inhibitsControl( false )
{
}

bool UI_Shell::Init()
{
	_ui = ( RmlUserInterfaceLocal* )rmlManager->Find( "main", true );
	_inhibitsControl = true;
	_isActive = true;
	_isPausingGame = false;

	// Register event listeners. Note that registering event listeners must happen before documents are loaded.
	Rml::Factory::RegisterEventListenerInstancer( &_eventListenerInstancer );

	if( LoadDocument( "startmenu" ) )
	{
		// Seed the initial materials.
		_ui->Redraw( 0 );
		return true;
	}

	return false;
}

void UI_Shell::Redraw( int time )
{
	if( !rmlManager->InLevelLoad() )
	{
		TransitionNextScreen();
	}
	_ui->Redraw( time );
}

void UI_Shell::SetNextScreen( const char* screen )
{
	_nextScreen = screen;
}

void UI_Shell::TransitionNextScreen()
{
	if( !_nextScreen.IsEmpty() )
	{
		LoadDocument( _nextScreen.c_str() );
		_nextScreen.Empty();
	}
}

Rml::ElementDocument* UI_Shell::LoadDocument( const char* windowName )
{
	// Set the event handler for the new screen, if one has been registered.
//EventHandler* old_event_handler = event_handler;
//EventHandlerMap::iterator iterator = event_handlers.find(window_name);
//if (iterator != event_handlers.end())
//	event_handler = (*iterator).second;
//else
//	event_handler = nullptr;

// Attempt to load the referenced RML document.
	idStr docPath = "guis/rml/shell/";
	docPath.AppendPath( windowName );
	docPath.Append( ".rml" );

	Rml::ElementDocument* document = _ui->Context()->LoadDocument( docPath.c_str() );
	if( document == nullptr )
	{
		//event_handler = old_event_handler;
		common->Warning( "Failed to load document %s\n", docPath.c_str() );
		return nullptr;
	}

	// Set the element's title on the title; IDd 'title' in the RML.
	Rml::Element* title = document->GetElementById( "title" );
	if( title != nullptr )
	{
		title->SetInnerRML( document->GetTitle() );
	}

	document->Show();

	return document;
}

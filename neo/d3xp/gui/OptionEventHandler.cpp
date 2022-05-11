#include "precompiled.h"
#pragma hdrstop

#include "EventHandlers.h"

#include "../Game_local.h"

#include "RmlUi/Core.h"
#include "RmlUi/Core/EventListener.h"

class UI_Shell;


void EventHandlerOptions::ProcessEvent( Rml::Event& _event, idLexer& _src, idToken& _token )
{
	if( !_token.Icmp( "restore" ) )
	{
		Rml::ElementDocument* optionsBody = _event.GetTargetElement( )->GetOwnerDocument( );

		if( !optionsBody )
		{
			return;
		}

		Rml::String windowModeId;
		switch( r_fullscreen.GetInteger( ) )
		{
			case 0: // windowed
				windowModeId = "windowed";
				break;
			case 1: // fullscreen on primary monitor
				windowModeId = "fullscreen";
				break;
			case 2: // fullscreen on secondary monitor
				windowModeId = "windowed_borderless";
				break;
		}

		Rml::ElementFormControlInput* windowModeOption = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( windowModeId ) );

		if( windowModeOption )
		{
			windowModeOption->SetAttribute( "checked", "" );
		}

		Rml::ElementFormControlInput* accept = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "accept" ) );
		Rml::ElementFormControlInput* apply = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "apply" ) );

		if( accept )
		{
			accept->SetDisabled( true );
		}

		if( apply )
		{
			apply->SetDisabled( true );
		}

		return;
	}

	if( !_token.Icmp( "store" ) )
	{
		const Rml::String subParm = _event.GetParameter<Rml::String>( "submit", "cancel" );
		if( subParm == "accept" || subParm == "apply" )
		{
			Rml::String windowMode = _event.GetParameter<Rml::String>( "window_mode", "fullscreen" );

			if( windowMode == "fullscreen" )
			{
				r_fullscreen.SetInteger( 1 );
			}
			else if( windowMode == "windowed" )
			{
				r_fullscreen.SetInteger( 0 );
			}
			else if( windowMode == "windowed_borderless" )
			{
				r_fullscreen.SetInteger( 2 );
			}

			int vidModeOption = _event.GetParameter<int>( "vid_mode", 0 );

			r_vidMode.SetInteger( vidModeOption );

			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "vid_restart\n" );
		}

		if( subParm == "cancel" || subParm == "accept" )
		{
			_event.GetTargetElement( )->GetOwnerDocument( )->Hide( );
			shell->HideScreen( "options" );
			if( shell->State( ) == ShellState::START )
			{
				shell->ShowScreen( "startmenu" );
			}
			else if( shell->State( ) == ShellState::GAME )
			{
				shell->ShowScreen( "game" );
			}
		}

		return;
	}

	if( !_token.Icmp( "enable_accept" ) )
	{
		Rml::ElementDocument* optionsBody = _event.GetTargetElement( )->GetOwnerDocument( );

		if( optionsBody == nullptr )
		{
			return;
		}

		// Enable the accept button when values are changed
		Rml::ElementFormControlInput* accept = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "accept" ) );
		Rml::ElementFormControlInput* apply = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "apply" ) );

		if( accept )
		{
			accept->SetDisabled( false );
		}

		if( apply )
		{
			apply->SetDisabled( false );
		}

		return;
	}

	RmlGameEventHandler::ProcessEvent( _event, _src, _token );
}
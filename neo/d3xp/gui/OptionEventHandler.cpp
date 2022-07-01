#include "precompiled.h"
#pragma hdrstop

#include "EventHandlers.h"

#include "../Game_local.h"

#include "RmlShell.h"

#include "RmlUi/Core.h"
#include "RmlUi/Core/EventListener.h"

class UI_Shell;


void EventHandlerOptions::ProcessEvent( Rml::Event& event, idLexer& src, idToken& token )
{
	if( !token.Icmp( "restore" ) )
	{
		Rml::ElementDocument* optionsBody = event.GetTargetElement( )->GetOwnerDocument( );

		if( !optionsBody )
		{
			return;
		}

		const Rml::String windowModeId = r_fullscreen.GetString();
		auto* windowModeOption = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(
			optionsBody->GetElementById( va( "option%s", windowModeId.c_str() ) ) );

		if( windowModeOption )
		{
			windowModeOption->SetAttribute( "checked", "" );
		}

		const auto accept = rmlui_dynamic_cast<Rml::ElementFormControlInput*>( optionsBody->GetElementById( "accept" ) );
		const auto apply = rmlui_dynamic_cast<Rml::ElementFormControlInput*>( optionsBody->GetElementById( "apply" ) );

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

	if( !token.Icmp( "store" ) )
	{
		const auto subParm = event.GetParameter<Rml::String>( "submit", "cancel" );
		if( subParm == "accept" || subParm == "apply" )
		{
			const auto windowMode = event.GetParameter<Rml::String>( "window_mode", "1" );
			const int windowIndex = event.GetParameter<int>( "window_size", 0 );
			const int displayIndex = event.GetParameter<int>( "display_hz", 0 );
			const int vidMode = shell->FindVidModeIndex( windowIndex, displayIndex );

			if( vidMode > -1 )
			{
				idList<vidMode_t> modeList;
				R_GetModeListForDisplay( 0, modeList );
				r_fullscreen.SetString( windowMode.c_str() );
				r_vidMode.SetInteger( vidMode );
				r_windowWidth.SetInteger( modeList[vidMode].width );
				r_windowHeight.SetInteger( modeList[vidMode].height );
				cvarSystem->ClearModifiedFlags( CVAR_ARCHIVE );
				cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "vid_restart\n" );
			}
		}

		if( subParm == "cancel" || subParm == "accept" )
		{
			event.GetTargetElement( )->GetOwnerDocument( )->Hide( );
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

		Rml::ElementDocument* optionsBody = event.GetTargetElement()->GetOwnerDocument();

		if (optionsBody == nullptr)
		{
			return;
		}

		// Enable the accept button when values are changed
		const auto accept = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(optionsBody->GetElementById("accept"));
		const auto apply = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(optionsBody->GetElementById("apply"));

		if (accept)
		{
			accept->SetDisabled(true);
		}

		if (apply)
		{
			apply->SetDisabled(true);
		}

		return;
	}

	if( !token.Icmp( "enable_accept" ) )
	{
		Rml::ElementDocument* optionsBody = event.GetTargetElement( )->GetOwnerDocument( );

		if( optionsBody == nullptr )
		{
			return;
		}

		// Enable the accept button when values are changed
		const auto accept = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "accept" ) );
		const auto apply = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "apply" ) );

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

	RmlGameEventHandler::ProcessEvent( event, src, token );
}
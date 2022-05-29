#include "precompiled.h"
#pragma hdrstop

#include "EventHandlers.h"
#include "RmlShell.h"

#include "../Game_local.h"

void RmlGameEventHandler::ProcessEvent( Rml::Event& _event, idLexer& _src, idToken& _token )
{
	if( _event == Rml::EventId::Keydown )
	{
		Rml::Input::KeyIdentifier keyId = ( Rml::Input::KeyIdentifier )_event.GetParameter< int >( "key_identifier", 0 );
		if( keyId == Rml::Input::KI_ESCAPE && _event.GetTargetElement( )->GetOwnerDocument( )->IsVisible( ) )
		{
			if( !_token.Icmp( "goto" ) )
			{
				_src.ReadToken( &_token );

				_event.GetTargetElement( )->GetOwnerDocument( )->Hide( );

				shell->SetNextScreen( _token.c_str( ) );
			}
		}
	}
	else if( !_token.Icmp( "goto" ) )
	{
		_src.ReadToken( &_token );

		_event.GetTargetElement( )->GetOwnerDocument( )->Hide( );

		shell->ShowScreen( _token.c_str( ) );
	}
}
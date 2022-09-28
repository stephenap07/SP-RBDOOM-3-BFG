#include "precompiled.h"
#pragma hdrstop

#include "EventHandlers.h"
#include "RmlShell.h"

#include "../Game_local.h"

static void DeactivateShell( UI_Shell* shell )
{
	shell->Ui()->Activate( false );
	shell->Ui()->SetIsPausingGame( false );
}

void RmlGameEventHandler::ProcessEvent( Rml::Event& event, idLexer& _src, idToken& _token )
{
	if( event == Rml::EventId::Keydown )
	{
		Rml::Input::KeyIdentifier keyId = ( Rml::Input::KeyIdentifier )event.GetParameter< int >( "key_identifier", 0 );
		if( keyId == Rml::Input::KI_ESCAPE && event.GetTargetElement( )->GetOwnerDocument( )->IsVisible( ) )
		{
			if( !_token.Icmp( "goto" ) )
			{
				_src.ReadToken( &_token );

				event.GetTargetElement( )->GetOwnerDocument( )->Hide( );

				shell->SetNextScreen( _token.c_str( ) );
			}

			if( !_token.Icmp( "deactivate" ) )
			{
				DeactivateShell( shell );
			}
		}
	}
	else if( !_token.Icmp( "goto" ) )
	{
		_src.ReadToken( &_token );

		event.GetTargetElement( )->GetOwnerDocument( )->Hide( );

		shell->SetNextScreen( _token.c_str( ) );
	}
	else if( !_token.Icmp( "deactivate" ) )
	{
		DeactivateShell( shell );
	}
}
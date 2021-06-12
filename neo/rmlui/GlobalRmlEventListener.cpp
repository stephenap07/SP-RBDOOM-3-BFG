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

#include "GlobalRmlEventListener.h"

GlobalRmlEventListener::GlobalRmlEventListener( RmlUserInterface* _ui, const Rml::String& value, Rml::Element* element )
	: ui( _ui )
	, value( value )
	, element( element )
{
}

void GlobalRmlEventListener::ProcessEvent( Rml::Event& event )
{
	using namespace Rml;

	idLexer src;
	idToken token;
	src.LoadMemory( value.c_str(), value.length(), "rmlCommands" );
	src.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );

	int width = ui->GetScreenSize().x;
	int height = ui->GetScreenSize().y;

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
			ui->SetNextScreen( token.c_str() );
			event.GetTargetElement()->GetOwnerDocument()->Hide();
			ui->CloseDocument( event.GetTargetElement()->GetOwnerDocument()->GetSourceURL().c_str() );
		}

		if( !token.Icmp( "load" ) )
		{
			src.ReadToken( &token );
			ui->SetNextScreen( token.c_str() );
		}

		if( !token.Icmp( "map" ) )
		{
			src.ReadToken( &token );
			cmdSystem->AppendCommandText( va( "map %s\n", token.c_str() ) );
		}

		if( !token.Icmp( "playsound" ) )
		{
			src.ReadToken( &token );
			ui->PlaySound( token.c_str() );
		}

		if( !token.Icmp( "localSound" ) )
		{
			src.ReadToken( &token );
			common->SW()->PlayShaderDirectly( token.c_str() );
		}

		if( !token.Icmp( "inhibitControl" ) )
		{
			src.ReadToken( &token );
			if( !token.Cmp( "=" ) )
			{
				src.ReadToken( &token );
			}
			if( !token.Icmp( "true" ) )
			{
				ui->SetInhibitsControl( true );
			}
			else if( !token.Icmp( "false" ) )
			{
				ui->SetInhibitsControl( false );
			}
		}

		if( !token.Icmp( "pauseGame" ) )
		{
			src.ReadToken( &token );
			if( !token.Cmp( "=" ) )
			{
				src.ReadToken( &token );
			}
			if( !token.Icmp( "true" ) )
			{
				ui->SetIsPausingGame( true );
			}
			else if( !token.Icmp( "false" ) )
			{
				ui->SetIsPausingGame( false );
			}
		}

		if( !token.Icmp( "enableCursor" ) )
		{
			src.ReadToken( &token );
			if( !token.Cmp( "=" ) )
			{
				src.ReadToken( &token );
			}
			if( !token.Icmp( "true" ) )
			{
				ui->SetCursorEnabled( true );
			}
			else if( !token.Icmp( "false" ) )
			{
				ui->SetCursorEnabled( false );
			}
		}

		if( !token.Icmp( "set" ) )
		{
			// Set internal variables
			src.ReadToken( &token );
			if( !token.Icmp( "windowWidth" ) )
			{
				src.ReadToken( &token );
				if( !token.Icmp( "=" ) )
				{
					// value
					ui->SetUseScreenResolution( false ); // use custom resolution
					src.ReadToken( &token );
					if( token.type == TT_NUMBER )
					{
						width = token.GetIntValue();
						ui->SetSize( width, height );
					}
				}
			}

			if( !token.Icmp( "windowHeight" ) )
			{
				src.ReadToken( &token );
				if( !token.Icmp( "=" ) )
				{
					// value
					ui->SetUseScreenResolution( false ); // use custom resolution
					src.ReadToken( &token );
					if( token.type == TT_NUMBER )
					{
						height = token.GetIntValue();
						ui->SetSize( width, height );
					}
				}
			}
		}
	}
}

void GlobalRmlEventListener::OnDetach( Rml::Element* element )
{
	delete this;
}


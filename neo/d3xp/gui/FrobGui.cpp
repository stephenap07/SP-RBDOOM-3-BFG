/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

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

#include <precompiled.h>
#pragma hdrstop

#include "FrobGui.h"

#include "d3xp/Game_local.h"
#include "rmlui/Core/ElementDocument.h"

FrobGui::FrobGui()
	: ui( nullptr )
	, mode( MODE_INVALID )
	, currentIcon( -1 )
{
}

void FrobGui::Init( idSoundWorld* soundWorld )
{
	ui = rmlManager->Find( "focus", false );
	ui->Init( "focus", soundWorld );

	doc = ui->LoadDocumentHandle( "guis/rml/hud/hud.rml" );
	const auto docElem = ui->GetDocumentFromHandle( doc );

	if( !docElem )
	{
		return;
	}

	docElem->Show();

	ui->Activate( true );

	SetMode( MODE_NONE );
}

void FrobGui::Redraw() const
{
	if( mode == MODE_NONE )
	{
		return;
	}

	ui->Redraw( gameLocal.GetTime() );
}

void FrobGui::Show( const bool show )
{
}

void FrobGui::SetMode( const Mode newMode, const idEntity* ent )
{
	if( mode == newMode )
	{
		return;
	}

	mode = newMode;

	switch( mode )
	{
		case MODE_NONE:
		{
			ShowText( false );
			ShowInteract( false );
			break;
		}

		case MODE_INTERACT:
		{
			ShowText( false );
			if( ent )
			{
				SetIcon( ent->spawnArgs.GetString( "interactIcon", "ui/assets/lightbulb" ) );
			}
			ShowInteract( true );
			break;
		}

		case MODE_SHOW_TEXT:
		{
			if( ent )
			{
				SetText( ent->spawnArgs.GetString( "flavorText", "<insert flavor text>" ) );
			}

			ShowText( true );
			ShowInteract( false );
			break;
		}

		case MODE_INVALID:
		{
			break;
		}

		default:
		{
			gameLocal.Warning( "Invalid newMode %d", mode );
		}
	}
}

void FrobGui::ShowText( const bool show )
{
	const auto docElem = ui->GetDocumentFromHandle( doc );
	docElem->Show();

	if( Rml::Element* elem = docElem->GetElementById( "text" ) )
	{
		if( show )
		{
			if( elem->IsClassSet( "fadeout" ) || elem->IsClassSet( "invisible" ) )
			{
				elem->SetClass( "fadeout", false );
				elem->SetClass( "fadein", true );
				elem->SetClass( "invisible", false );
			}
		}
		else
		{
			if( elem->IsClassSet( "fadein" ) )
			{
				elem->SetClass( "fadein", false );
				elem->SetClass( "fadeout", true );
			}
		}
	}
}

void FrobGui::ShowInteract( const bool show )
{
	const auto docElem = ui->GetDocumentFromHandle( doc );
	docElem->Show();

	if( Rml::Element* elem = docElem->GetElementById( "interact" ) )
	{
		if( show )
		{
			if( elem->IsClassSet( "fadeout" ) || elem->IsClassSet( "invisible" ) )
			{
				elem->SetClass( "fadeout", false );
				elem->SetClass( "fadein", true );
				elem->SetClass( "invisible", false );
			}
		}
		else
		{
			if( elem->IsClassSet( "fadein" ) )
			{
				elem->SetClass( "fadein", false );
				elem->SetClass( "fadeout", true );
			}
		}
	}
}

void FrobGui::SetText( const char* text )
{
	if( const auto docElem = ui->GetDocumentFromHandle( doc ) )
	{
		docElem->Show();
		if( Rml::Element* elem = docElem->GetElementById( "text-content" ) )
		{
			elem->SetInnerRML( text );
		}
	}
}

void FrobGui::SetIcon( const char* materialName )
{
	const idMaterial* mat = nullptr;

	const auto h = idStr::Hash( materialName );
	iconHashIndex.GetFirst( idStr::Hash( materialName ) );

	for( int i = iconHashIndex.First( h ); i >= 0; i = iconHashIndex.Next( i ) )
	{
		if( idStr::Icmp( icons[i]->GetName(), materialName ) == 0 )
		{
			if( currentIcon == i )
			{
				return;
			}

			currentIcon = i;

			mat = icons[i];
			break;
		}
	}

	if( !mat )
	{
		mat = declManager->FindMaterial( materialName );
		if( mat )
		{
			iconHashIndex.Add( h, icons.Append( { mat } ) );
		}
	}

	if( mat )
	{
		if( const auto docElem = ui->GetDocumentFromHandle( doc ) )
		{
			if( Rml::Element* elem = docElem->GetElementById( "interact" ) )
			{
				elem->SetAttribute( "src", ( idStr( "/" ) + mat->GetName() ).c_str() );
			}
		}
	}
}

void FrobGui::ToggleView( const idEntity* entity )
{
	if( mode == MODE_INTERACT )
	{
		SetMode( MODE_SHOW_TEXT, entity );
	}
	else
	{
		SetMode( MODE_INTERACT, entity );
	}
}

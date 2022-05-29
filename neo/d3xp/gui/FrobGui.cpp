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
	: _ui( nullptr )
	, _doc( nullptr )
	, _mode( MODE_INVALID )
{
}

void FrobGui::Init( idSoundWorld* soundWorld_ )
{
	_ui = rmlManager->Find( "focus", false );
	_ui->Init( "focus", soundWorld_ );

	if( _ui )
	{
		_doc = _ui->LoadDocument( "guis/rml/hud/hud.rml" );
		if( _doc )
		{
			_doc->Show();
		}
	}

	_ui->Activate( true );

	{
		if( Rml::Element* elem = _doc->GetElementById( "text" ) )
		{
			if( !elem->IsClassSet( "fadeout" ) && !elem->IsClassSet( "fadein" ) )
			{
				elem->SetClass( "fadeout", true );
			}
		}
	}

	{
		if( Rml::Element* elem = _doc->GetElementById( "interact" ) )
		{
			if( !elem->IsClassSet( "fadeout" ) && !elem->IsClassSet( "fadein" ) )
			{
				elem->SetClass( "fadeout", true );
			}
		}
	}

	SetMode( MODE_NONE );
}

void FrobGui::Redraw()
{
	_ui->Redraw( gameLocal.time );
}

void FrobGui::Show( const bool show_ )
{
	if( show_ )
	{
		_doc->Show();
	}
	else
	{
		_doc->Hide();
	}
}

void FrobGui::SetMode( const Mode mode_ )
{
	if( _mode == mode_ )
	{
		return;
	}

	_mode = mode_;

	switch( _mode )
	{
		case Mode::MODE_NONE:
		{
			ShowText( false );
			ShowInteract( false );
			break;
		}

		case Mode::MODE_INTERACT:
		{
			ShowText( false );
			ShowInteract( true );
			break;
		}

		case Mode::MODE_SHOW_TEXT:
		{
			ShowText( true );
			ShowInteract( false );
			break;
		}

		default:
		{
			gameLocal.Warning( "Invalid mode %d", _mode );
		}
	}
}

void FrobGui::ShowText( const bool show_ )
{
	Rml::Element* elem = _doc->GetElementById( "text" );
	if( elem )
	{
		if( show_ )
		{
			if( elem->IsClassSet( "fadeout" ) )
			{
				elem->SetClass( "fadeout", false );
				elem->SetClass( "fadein", true );
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

void FrobGui::ShowInteract( const bool show_ )
{
	Rml::Element* elem = _doc->GetElementById( "interact" );
	if( elem )
	{
		if( show_ )
		{
			if( elem->IsClassSet( "fadeout" ) )
			{
				elem->SetClass( "fadeout", false );
				elem->SetClass( "fadein", true );
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

void FrobGui::SetText( const char* text_ )
{
	if( _doc )
	{
		Rml::Element* elem = _doc->GetElementById( "text-content" );
		if( elem )
		{
			elem->SetInnerRML( text_ );
		}
	}
}

void FrobGui::ToggleView( const idEntity* entity_ )
{
	if( _mode == MODE_INTERACT )
	{
		if( entity_ )
		{
			SetText( entity_->spawnArgs.GetString( "flavorText", "<insert flavor text>" ) );
		}

		SetMode( FrobGui::MODE_SHOW_TEXT );
	}
	else
	{
		SetMode( FrobGui::MODE_INTERACT );
	}
}

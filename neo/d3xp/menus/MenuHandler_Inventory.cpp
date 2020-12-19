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
#pragma hdrstop
#include "precompiled.h"
#include "../Game_local.h"


idMenuHandler_Inventory::~idMenuHandler_Inventory()
{
	commandBarWidget.Cleanup();
}

void idMenuHandler_Inventory::Update()
{
	if( gui == NULL || !gui->IsActive() )
	{
		return;
	}

	if( activeScreen != nextScreen )
	{
		if( nextScreen == INVENTORY_AREA_INVALID )
		{
			menuScreens[activeScreen]->HideScreen( static_cast<mainMenuTransition_t>( transition ) );

			idMenuWidget_CommandBar* cmdBar = dynamic_cast<idMenuWidget_CommandBar*>( GetChildFromIndex( 0 ) );
			if( cmdBar != NULL )
			{
				cmdBar->ClearAllButtons();
				cmdBar->Update();
			}

			idSWFSpriteInstance* bg = gui->GetRootObject().GetNestedSprite( "background" );
			idSWFSpriteInstance* edging = gui->GetRootObject().GetNestedSprite( "_fullScreen" );

			if( bg != NULL )
			{
				bg->PlayFrame( "rollOff" );
			}

			if( edging != NULL )
			{
				edging->PlayFrame( "rollOff" );
			}
		}
		else
		{
			if( activeScreen > INVENTORY_AREA_INVALID && activeScreen < INVENTORY_NUM_AREAS && menuScreens[activeScreen] != NULL )
			{
				menuScreens[activeScreen]->HideScreen( static_cast<mainMenuTransition_t>( transition ) );
			}

			if( nextScreen > INVENTORY_AREA_INVALID && nextScreen < INVENTORY_NUM_AREAS && menuScreens[nextScreen] != NULL )
			{
				menuScreens[nextScreen]->UpdateCmds();
				menuScreens[nextScreen]->ShowScreen( static_cast<mainMenuTransition_t>( transition ) );
			}
		}

		transition = MENU_TRANSITION_INVALID;
		activeScreen = nextScreen;
	}

	idMenuHandler::Update();
}

void idMenuHandler_Inventory::ActivateMenu( bool show )
{
	idMenuHandler::ActivateMenu( show );

	if( show )
	{
		idPlayer* player = gameLocal.GetLocalPlayer();
		if( player == NULL )
		{
			return;
		}

		idMenuWidget_CommandBar* cmdBar = dynamic_cast<idMenuWidget_CommandBar*>( GetChildFromIndex( 0 ) );
		if( cmdBar != NULL )
		{
			cmdBar->ClearAllButtons();
			cmdBar->Update();
		}
	}
	else
	{
		activeScreen = INVENTORY_AREA_INVALID;
		nextScreen = INVENTORY_AREA_INVALID;
	}

}


void idMenuHandler_Inventory::Initialize( const char* swfFile, idSoundWorld* sw )
{
	idMenuHandler::Initialize( swfFile, sw );

	//---------------------
	// Initialize the menus
	//---------------------
#define BIND_INVENTORY_SCREEN( screenId, className, menuHandler )				\
	menuScreens[ (screenId) ] = new className();						\
	menuScreens[ (screenId) ]->Initialize( menuHandler );				\
	menuScreens[ (screenId) ]->AddRef();

	for( int i = 0; i < INVENTORY_NUM_AREAS; ++i )
	{
		menuScreens[i] = NULL;
	}

	BIND_INVENTORY_SCREEN( INVENTORY_AREA_PLAYING, idMenuScreen_Inventory, this );

	class idInventoryClose : public idSWFScriptFunction_RefCounted
	{
	public:
		idSWFScriptVar Call( idSWFScriptObject* thisObject, const idSWFParmList& parms )
		{
			return idSWFScriptVar();
		}
	};

	if( gui != NULL )
	{
		gui->SetGlobal( "closeMenu", new idInventoryClose() );
	}

	//
	// nav bar
	//
	navBar.SetSpritePath( "navBar", "options" );
	navBar.Initialize( this );
	navBar.SetNumVisibleOptions( 4 ); // TODO replace
	navBar.SetWrappingAllowed( true );
	navBar.SetButtonSpacing( 20.0f, 25.0f, 75.0f );
	navBar.SetInitialXPos( 40.0f );
	navBar.SetNoAutoFree( true );
	for( int count = 0; count < ( 4 * 2 - 1 ); ++count )
	{
		idMenuWidget_NavButton* const navButton = new( TAG_SWF ) idMenuWidget_NavButton();

		if( count < 4 - 1 )
		{
			navButton->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK, count );
		}
		else if( count < ( ( 4 - 1 ) * 2 ) )
		{
			int index = ( count - ( 4 - 1 ) ) + 1;
			navButton->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK, index );
		}
		else
		{
			navButton->AddEventAction( WIDGET_EVENT_PRESS ).Set( WIDGET_ACTION_GO_BACK, -1 );
		}

		navBar.AddChild( navButton );
	}

	//
	// command bar
	//
	commandBarWidget.SetAlignment( idMenuWidget_CommandBar::LEFT );
	commandBarWidget.SetSpritePath( "prompts" );
	commandBarWidget.Initialize( this );
	commandBarWidget.SetNoAutoFree( true );

	AddChild( &commandBarWidget );
}

bool idMenuHandler_Inventory::HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled )
{
	if( activeScreen == INVENTORY_AREA_INVALID )
	{
		return true;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	if( event.type == WIDGET_EVENT_COMMAND )
	{
		if( menuScreens[activeScreen] != NULL && !forceHandled )
		{
			if( menuScreens[activeScreen]->HandleAction( action, event, widget, true ) )
			{
				if( actionType == WIDGET_ACTION_GO_BACK )
				{
					PlaySound( GUI_SOUND_BACK );
				}
				else
				{
					PlaySound( GUI_SOUND_ADVANCE );
				}
				return true;
			}
		}
	}

	return idMenuHandler::HandleAction( action, event, widget, forceHandled );
}


idMenuScreen* idMenuHandler_Inventory::GetMenuScreen( int index )
{
	if( index < 0 || index >= INVENTORY_NUM_AREAS )
	{
		return NULL;
	}

	return menuScreens[index];

}

idMenuScreen_Inventory* idMenuHandler_Inventory::GetInventory()
{
	idMenuScreen_Inventory* screen = dynamic_cast<idMenuScreen_Inventory*>( menuScreens[INVENTORY_AREA_PLAYING] );
	return screen;
}
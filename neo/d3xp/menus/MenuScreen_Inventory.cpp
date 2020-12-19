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



idMenuScreen_Inventory::idMenuScreen_Inventory()
	: testButton( nullptr )
{
}


idMenuScreen_Inventory::~idMenuScreen_Inventory()
{

}

void idMenuScreen_Inventory::Initialize( idMenuHandler* data )
{
	idMenuScreen::Initialize( data );

	if( data != NULL )
	{
		menuGUI = data->GetGUI();
	}

	SetSpritePath( "menuItems" );
}

void idMenuScreen_Inventory::Update()
{
	idPlayer* player = gameLocal.GetLocalPlayer();
	if( player == NULL )
	{
		idMenuScreen::Update();
		return;
	}

	if( menuData != NULL )
	{
		idMenuWidget_CommandBar* cmdBar = dynamic_cast<idMenuWidget_CommandBar* const>( menuData->GetChildFromIndex( 0 ) );
		if( cmdBar != NULL )
		{
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_JOY2 );
			if( menuData->GetPlatform() != 2 )
			{
				buttonInfo->label = "#str_01345";
			}
			buttonInfo->action.Set( WIDGET_ACTION_GO_BACK );

			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_JOY3 );
			buttonInfo->label = "#str_SWF_EQUIP";
			buttonInfo->action.Set( WIDGET_ACTION_JOY3_ON_PRESS );

			buttonInfo = cmdBar->GetButton( idMenuWidget_CommandBar::BUTTON_TAB );
			buttonInfo->label = "";
			buttonInfo->action.Set( WIDGET_ACTION_GO_BACK );
		}
	}

	idMenuScreen::Update();
}

void idMenuScreen_Inventory::ShowScreen( const mainMenuTransition_t transitionType )
{
	if( menuGUI != NULL && menuData != NULL )
	{
		idSWFScriptObject& root = menuGUI->GetRootObject();
		idSWFSpriteInstance* navBar = root.GetNestedSprite( "menuItems" );
		if( navBar != NULL && menuData != NULL && menuData->ActiveScreen() == INVENTORY_AREA_INVALID )
		{
			navBar->PlayFrame( "rollOn" );

			auto txtVal = navBar->GetScriptObject()->GetNestedText( "info", "pdaData", "txtName" );

			if( txtVal )
			{
				txtVal->SetText( "Hello, World!!" );
			}

			auto txtHeader = root.GetNestedText( "titleContainer", "txtVal" );

			if( txtHeader )
			{
				txtHeader->SetText( "Inventory" );
			}
		}
	}

	idMenuScreen::ShowScreen( transitionType );
}

void idMenuScreen_Inventory::HideScreen( const mainMenuTransition_t transitionType )
{
	if( menuGUI != NULL )
	{
		idSWFScriptObject& root = menuGUI->GetRootObject();
		idSWFSpriteInstance* pdaSprite = root.GetNestedSprite( "menuItems" );
		if( pdaSprite != NULL && menuData != NULL && menuData->NextScreen() == INVENTORY_AREA_INVALID )
		{
			pdaSprite->SetVisible( true );
			pdaSprite->PlayFrame( "rollOff" );
		}
	}

	idMenuScreen::HideScreen( transitionType );
}

bool idMenuScreen_Inventory::HandleAction( idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled )
{

	if( menuData == NULL )
	{
		return true;
	}

	if( menuData->ActiveScreen() != INVENTORY_AREA_PLAYING )
	{
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	switch( actionType )
	{
		case WIDGET_ACTION_JOY3_ON_PRESS:
		{
			menuData->SetNextScreen( PDA_AREA_INVALID, MENU_TRANSITION_ADVANCE );
			return true;
		}
		case WIDGET_ACTION_GO_BACK:
		{
			menuData->SetNextScreen( PDA_AREA_INVALID, MENU_TRANSITION_ADVANCE );
			return true;
		}
		case WIDGET_ACTION_START_REPEATER:
		{
			idWidgetAction repeatAction;
			widgetAction_t repeatActionType = static_cast<widgetAction_t>( parms[0].ToInteger() );
			assert( parms.Num() == 2 );
			repeatAction.Set( repeatActionType, parms[1] );
			menuData->StartWidgetActionRepeater( widget, repeatAction, event );
			return true;
		}
	}

	return idMenuWidget::HandleAction( action, event, widget, forceHandled );
}

void idMenuScreen_Inventory::HandleMenu( const mainMenuTransition_t type )
{
	idMenuScreen::HandleMenu( type );
}
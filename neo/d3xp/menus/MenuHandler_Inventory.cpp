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


void idMenuHandler_Inventory::Update()
{
	if (gui == NULL || !gui->IsActive())
	{
		return;
	}

	if (activeScreen != nextScreen)
	{
		if (nextScreen == INVENTORY_AREA_INVALID)
		{
			menuScreens[activeScreen]->HideScreen(static_cast<mainMenuTransition_t>(transition));
		}
		else
		{
			if (activeScreen > INVENTORY_AREA_INVALID && activeScreen < INVENTORY_NUM_AREAS && menuScreens[activeScreen] != NULL)
			{
				menuScreens[activeScreen]->HideScreen(static_cast<mainMenuTransition_t>(transition));
			}

			if (nextScreen > INVENTORY_AREA_INVALID && nextScreen < INVENTORY_NUM_AREAS && menuScreens[nextScreen] != NULL)
			{
				menuScreens[nextScreen]->UpdateCmds();
				menuScreens[nextScreen]->ShowScreen(static_cast<mainMenuTransition_t>(transition));
			}
		}

		transition = MENU_TRANSITION_INVALID;
		activeScreen = nextScreen;
	}

	idMenuHandler::Update();
}

void idMenuHandler_Inventory::ActivateMenu(bool show)
{
	idMenuHandler::ActivateMenu(show);

	idPlayer* player = gameLocal.GetLocalPlayer();
	if (player == NULL)
	{
		return;
	}

	if (show)
	{
		activeScreen = INVENTORY_AREA_INVALID;
		nextScreen = INVENTORY_AREA_PLAYING;
	}
	else
	{
		activeScreen = INVENTORY_AREA_INVALID;
		nextScreen = INVENTORY_AREA_INVALID;
	}

}


void idMenuHandler_Inventory::Initialize(const char* swfFile, idSoundWorld* sw)
{
	idMenuHandler::Initialize(swfFile, sw);

	//---------------------
	// Initialize the menus
	//---------------------
#define BIND_INVENTORY_SCREEN( screenId, className, menuHandler )				\
	menuScreens[ (screenId) ] = new className();						\
	menuScreens[ (screenId) ]->Initialize( menuHandler );				\
	menuScreens[ (screenId) ]->AddRef();

	for (int i = 0; i < INVENTORY_NUM_AREAS; ++i)
	{
		menuScreens[i] = NULL;
	}

	BIND_INVENTORY_SCREEN(INVENTORY_AREA_PLAYING, idMenuScreen_Inventory, this);
}


idMenuScreen* idMenuHandler_Inventory::GetMenuScreen(int index)
{
	if (index < 0 || index >= INVENTORY_NUM_AREAS)
	{
		return NULL;
	}

	return menuScreens[index];

}

idMenuScreen_Inventory* idMenuHandler_Inventory::GetInventory()
{
	idMenuScreen_Inventory* screen = dynamic_cast<idMenuScreen_Inventory*>(menuScreens[INVENTORY_AREA_PLAYING]);
	return screen;
}
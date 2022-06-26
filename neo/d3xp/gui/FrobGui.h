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

#ifndef GAME_GUI_FROBGUI_H
#define GAME_GUI_FROBGUI_H

#include "rmlui/RmlUserInterface.h"


/**
 * \brief A gui to display text on the bottom center of the screen.
 * A 'Frob' is a word made up by the Thief developers to indicate something
 * that can be interacted with.
 *
 * "The player frobbed the computer and it turned on."
 */
class FrobGui
{
public:

	/**
	 * \brief Current mode of the Frob gui.
	 */
	enum Mode
	{
		MODE_INVALID,	//!< Initial mode to set in the class before initialization
		MODE_NONE,		//!< Do not show the elements
		MODE_INTERACT,	//!< Show the interact reticule in the center of the screen
		MODE_SHOW_TEXT	//!< Show the frob text on the center bottom of the screen
	};

	FrobGui();

	void Init( idSoundWorld* soundWorld );

	void Redraw() const;

	void Show( bool show );

	void SetMode( Mode newMode, const idEntity* ent = nullptr );

	void ShowText( bool show );

	void ShowInteract( bool show );

	void SetText( const char* text );

	void SetIcon( const char* materialName );

	/**
	 * \brief Toggle between @c MODE_INTERACT and @c MODE_SHOW_TEXT.
	 * \param entity Get the flavor text from the entity
	 */
	void ToggleView( const idEntity* entity );

	[[nodiscard]] Mode GetMode() const
	{
		return mode;
	}

private:

	RmlUserInterface*			ui;		//!< Reference to the control class for managing RML documents within the game
	RmlDocHandle				doc;	//!< Reference to the RML document to manipulate the visual RML document.
	Mode						mode;	//!< Current mode of the Frobbing.
	int							currentIcon;
	idHashIndex					iconHashIndex;
	idList<const idMaterial*>	icons;
};

#endif
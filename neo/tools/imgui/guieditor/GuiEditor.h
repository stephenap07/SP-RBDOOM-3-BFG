/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef EDITOR_TOOLS_GUIEDITOR_H_
#define EDITOR_TOOLS_GUIEDITOR_H_

#include "../../edit_public.h"

namespace ImGuiTools
{

struct MouseCmd
{
	short x;
	short y;
	bool pressed;
};

/**
* Gui imgui editor.
*/
class GuiEditor
{
public:
	virtual				~GuiEditor();

	void				Init();

	void				Draw();

	static GuiEditor&	Instance();

private:

	GuiEditor();

	void				Browse();

	void				HandleEvent();

	bool				isInitialized;
	int					openGui;
	idVec2				imgSize;
	idUserInterface*	gui;
	const idMaterial*	mat;

	idStrList			guiFiles;
	idStr				selectedGui;
};

}

#endif
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

#ifndef __RML_RMLSYSTEM_H_
#define __RML_RMLSYSTEM_H_

#include "RmlUi/Core/Input.h"
#include "RmlUi/Core/SystemInterface.h"

class idRmlSystem : public Rml::SystemInterface
{
public:

	enum
	{
		CURSOR_ARROW,
		CURSOR_HAND,
		CURSOR_HAND_JOY1,
		CURSOR_HAND_JOY2,
		CURSOR_HAND_JOY3,
		CURSOR_HAND_JOY4,
		CURSOR_COUNT
	};

	idRmlSystem();
	~idRmlSystem() override;

	/// Get the number of seconds elapsed since the start of the application.
	/// @return Elapsed time, in seconds.
	double GetElapsedTime() override;

	/// Log the specified message.
	/// @param[in] type Type of log message, ERROR, WARNING, etc.
	/// @param[in] message Message to log.
	/// @return True to continue execution, false to break into the debugger.
	bool LogMessage( Rml::Log::Type type, const Rml::String& message ) override;

	/// Set clipboard text.
	/// @param[in] text Text to apply to clipboard.
	void SetClipboardText( const Rml::String& text ) override;

	/// Get clipboard text.
	/// @param[out] text Retrieved text from clipboard.
	void GetClipboardText( Rml::String& text ) override;

	/// Set mouse cursor.
	/// @param[in] cursor_name Cursor name to activate.
	void SetMouseCursor( const Rml::String& cursorName ) override;

	int TranslateString(Rml::String& translated, const Rml::String& input) override;

	static Rml::Input::KeyIdentifier TranslateKey( int key );

	static int GetKeyModifier();

private:

	int _cursor = 0;
};

#endif
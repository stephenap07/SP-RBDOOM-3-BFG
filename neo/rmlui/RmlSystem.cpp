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

#include "RmlUserInterfaceLocal.h"

extern idDeviceContext* rmlDc;

/*
===============
idRmlSystem

Id system calls provided to rml
===============
*/

idRmlSystem::idRmlSystem()
{
}

idRmlSystem::~idRmlSystem()
{
}

double idRmlSystem::GetElapsedTime()
{
	// Might want to use some other clock.
	return static_cast<double>( Sys_Milliseconds() ) / 1000.0 / 2.0f;
}

bool idRmlSystem::LogMessage( Rml::Log::Type type, const Rml::String& message )
{
	if( type == Rml::Log::LT_ERROR )
	{
		common->Warning( "[RML] %s", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_INFO )
	{
		common->Printf( "[RML|INFO] %s", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_ASSERT )
	{
		common->Printf( "[RML|INFO] %s", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_ALWAYS )
	{
		common->Printf( "[RML|INFO] %s", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_DEBUG )
	{
		common->DPrintf( "[RML|INFO] %s", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_WARNING )
	{
		common->Warning( "[RML|INFO] %s", message.c_str() );
		return true;
	}

	return true;
}

void idRmlSystem::SetClipboardText( const Rml::String& text )
{
	Sys_SetClipboardData( text.c_str() );
}

void idRmlSystem::GetClipboardText( Rml::String& text )
{
	text = Sys_GetClipboardData();
}

void idRmlSystem::SetMouseCursor( const Rml::String& cursorName )
{
	if( cursorName.empty() || cursorName == "arrow" )
	{
		rmlDc->SetCursor( idDeviceContext::CURSOR_ARROW );
	}
	else if( cursorName == "move" )
	{
		rmlDc->SetCursor( idDeviceContext::CURSOR_HAND );
	}
	else if( cursorName == "pointer" )
	{
		rmlDc->SetCursor( idDeviceContext::CURSOR_ARROW );
	}
	else if( cursorName == "resize" )
	{
		rmlDc->SetCursor( idDeviceContext::CURSOR_HAND_JOY2 );
	}
	else if( cursorName == "cross" )
	{
		rmlDc->SetCursor( idDeviceContext::CURSOR_ARROW );
	}
	else if( cursorName == "text" )
	{
		rmlDc->SetCursor( idDeviceContext::CURSOR_HAND_JOY3 );
	}
	else if( cursorName == "unavailable" )
	{
		rmlDc->SetCursor( idDeviceContext::CURSOR_ARROW );
	}
}

Rml::Input::KeyIdentifier idRmlSystem::TranslateKey( int key )
{
	using namespace Rml::Input;

	switch( key )
	{
		case K_SPACE:
			return KI_SPACE;
		case K_0:
			return KI_0;
		case K_1:
			return KI_1;
		case K_2:
			return KI_2;
		case K_3:
			return KI_3;
		case K_4:
			return KI_4;
		case K_5:
			return KI_5;
		case K_6:
			return KI_6;
		case K_7:
			return KI_7;
		case K_8:
			return KI_8;
		case K_9:
			return KI_9;
		case K_A:
			return KI_A;
		case K_B:
			return KI_B;
		case K_C:
			return KI_C;
		case K_D:
			return KI_D;
		case K_E:
			return KI_E;
		case K_F:
			return KI_F;
		case K_G:
			return KI_G;
		case K_H:
			return KI_H;
		case K_I:
			return KI_I;
		case K_J:
			return KI_J;
		case K_K:
			return KI_K;
		case K_L:
			return KI_L;
		case K_M:
			return KI_M;
		case K_N:
			return KI_N;
		case K_O:
			return KI_O;
		case K_P:
			return KI_P;
		case K_Q:
			return KI_Q;
		case K_R:
			return KI_R;
		case K_S:
			return KI_S;
		case K_T:
			return KI_T;
		case K_U:
			return KI_U;
		case K_V:
			return KI_V;
		case K_W:
			return KI_W;
		case K_X:
			return KI_X;
		case K_Y:
			return KI_Y;
		case K_Z:
			return KI_Z;
		case K_SEMICOLON:
			return KI_OEM_1;
		case K_EQUALS:
			return KI_OEM_PLUS;
		case K_COMMA:
			return KI_OEM_COMMA;
		case K_MINUS:
			return KI_OEM_MINUS;
		case K_PERIOD:
			return KI_OEM_PERIOD;
		case K_SLASH:
			return KI_OEM_2;
			break;
		case K_GRAVE:
			return KI_OEM_3;
		case K_LBRACKET:
			return KI_OEM_4;
		case K_BACKSLASH:
			return KI_OEM_5;
		case K_RBRACKET:
			return KI_OEM_6;
		case K_APOSTROPHE:
			return KI_OEM_7;
		case K_KP_0:
			return KI_NUMPAD0;
		case K_KP_1:
			return KI_NUMPAD1;
		case K_KP_2:
			return KI_NUMPAD2;
		case K_KP_3:
			return KI_NUMPAD3;
		case K_KP_4:
			return KI_NUMPAD4;
		case K_KP_5:
			return KI_NUMPAD5;
			break;
		case K_KP_6:
			return KI_NUMPAD6;
		case K_KP_7:
			return KI_NUMPAD7;
		case K_KP_8:
			return KI_NUMPAD8;
		case K_KP_9:
			return KI_NUMPAD9;
		case K_KP_ENTER:
			return KI_NUMPADENTER;
		case K_KP_STAR:
			return KI_MULTIPLY;
		case K_KP_PLUS:
			return KI_ADD;
		case K_KP_MINUS:
			return KI_SUBTRACT;
		case K_KP_DOT:
			return KI_DECIMAL;
		case K_KP_SLASH:
			return KI_DIVIDE;
		case K_KP_EQUALS:
			return KI_OEM_NEC_EQUAL;
		case K_BACKSPACE:
			return KI_BACK;
		case K_TAB:
			return KI_TAB;
		case K_ENTER:
			return KI_RETURN;
		case K_PAUSE:
			return KI_PAUSE;
		case K_CAPSLOCK:
			return KI_CAPITAL;
		case K_PGUP:
			return KI_PRIOR;
		case K_PGDN:
			return KI_NEXT;
		case K_END:
			return KI_END;
		case K_HOME:
			return KI_HOME;
		case K_LEFTARROW:
			return KI_LEFT;
		case K_UPARROW:
			return KI_UP;
		case K_RIGHTARROW:
			return KI_RIGHT;
		case K_DOWNARROW:
			return KI_DOWN;
		case K_INS:
			return KI_INSERT;
		case K_DEL:
			return KI_DELETE;
		case K_F1:
			return KI_F1;
		case K_F2:
			return KI_F2;
		case K_F3:
			return KI_F3;
		case K_F4:
			return KI_F4;
		case K_F5:
			return KI_F5;
			break;
		case K_F6:
			return KI_F6;
		case K_F7:
			return KI_F7;
		case K_F8:
			return KI_F8;
		case K_F9:
			return KI_F9;
		case K_F10:
			return KI_F10;
		case K_F11:
			return KI_F11;
		case K_F12:
			return KI_F12;
		case K_F13:
			return KI_F13;
		case K_F14:
			return KI_F14;
		case K_F15:
			return KI_F15;
		case K_NUMLOCK:
			return KI_NUMLOCK;
		case K_SCROLL:
			return KI_SCROLL;
			break;
		case K_LSHIFT:
			return KI_LSHIFT;
		case K_RSHIFT:
			return KI_RSHIFT;
		case K_LCTRL:
			return KI_LCONTROL;
		case K_RCTRL:
			return KI_RCONTROL;
		case K_LALT:
			return KI_LMENU;
		case K_RALT:
			return KI_RMENU;
		case K_ESCAPE:
			return KI_ESCAPE;
		/*case SDLK_LSUPER:
			return KI_LWIN;
			break;
		case SDLK_RSUPER:
			return KI_RWIN;
			break;*/
		default:
			return KI_UNKNOWN;
	}
}

int idRmlSystem::GetKeyModifier()
{
	int keyModState = 0;

	if( idKeyInput::IsDown( K_CAPSLOCK ) )
	{
		keyModState |= Rml::Input::KM_CAPSLOCK;
	}

	if( idKeyInput::IsDown( K_LSHIFT ) || idKeyInput::IsDown( K_RSHIFT ) )
	{
		keyModState |= Rml::Input::KM_SHIFT;
	}

	if( idKeyInput::IsDown( K_NUMLOCK ) )
	{
		keyModState |= Rml::Input::KM_NUMLOCK;
	}

	if( idKeyInput::IsDown( K_LCTRL ) || idKeyInput::IsDown( K_RCTRL ) )
	{
		keyModState |= Rml::Input::KM_CTRL;
	}

	if( idKeyInput::IsDown( K_LALT ) || idKeyInput::IsDown( K_RALT ) )
	{
		keyModState |= Rml::Input::KM_ALT;
	}

	return keyModState;
}
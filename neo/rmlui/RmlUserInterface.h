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

#ifndef __RMLUSERINTERFACE_H__
#define __RMLUSERINTERFACE_H__

class RmlUi
{
public:
	virtual bool Reload() = 0;
	virtual bool InhibitsControl() = 0;
};

namespace Rml
{
class ElementDocument;
class Context;
}

class RmlEventHandler;

using RmlDocHandle = Handle;

// This class provides an interface to manage the rml documents. It provides access to reload documents and handle material loading as you edit them in real time.
class RmlUserInterface
{
public:
	virtual ~RmlUserInterface() = default;

	virtual bool					Init( const char* name, idSoundWorld* soundWorld ) = 0;

	// Handles an event, can return an action string, the caller interprets
	// any return and acts accordingly
	virtual const char*				HandleEvent( const sysEvent_t* event, int time ) = 0;

	// Handles a named event
	virtual void					HandleNamedEvent( const char* eventName ) = 0;

	virtual void					Reload( ) = 0;

	virtual void					ReloadStyleSheet( ) = 0;

	// Repaints the ui
	virtual void					Redraw( int time ) = 0;

	virtual RmlDocHandle			LoadDocumentHandle( const char* filePath, RmlEventHandler* eventHandler = nullptr ) = 0;

	virtual Rml::ElementDocument*	GetDocumentFromHandle( RmlDocHandle handle ) = 0;

	virtual bool					IsDocumentOpen( const char* name ) = 0;

	virtual void					CloseDocument( const char* name ) = 0;

	// Cursor
	virtual bool					IsCursorEnabled( ) const = 0;
	virtual void					SetCursorEnabled( bool _enabled = true ) = 0;

	virtual void					SetCursor( float x, float y ) = 0;

	virtual float					CursorX() = 0;

	virtual float					CursorY() = 0;

	// Activated the gui.
	virtual const char*				Activate( bool activate ) = 0;

	virtual Rml::Context*			Context() = 0;

	virtual bool					IsActive() = 0;

	virtual bool					IsPausingGame() = 0;

	virtual void					SetIsPausingGame( bool pause ) = 0;

	virtual bool					InhibitsControl() = 0;

	virtual void					SetInhibitsControl( bool inhibit ) = 0;

	virtual Rml::ElementDocument*	SetNextScreen( const char* _nextScreen, RmlEventHandler* _eventHandler = nullptr ) = 0;

	virtual void					HideAllDocuments() = 0;

	virtual Rml::ElementDocument*	GetDocument( const char* _path ) = 0;

	// Window specific

	virtual idVec2					GetScreenSize() const = 0;
	virtual void					SetSize( int width, int height ) = 0;
	virtual void					SetUseScreenResolution( bool useScreen ) = 0;

	// Sound
	virtual int						PlaySound( const char* sound, int channel = SCHANNEL_ANY, bool blocking = false ) = 0;

	virtual void					StopSound( int channel = SCHANNEL_ANY ) = 0;

	virtual void					AddCommand( const char* _cmd ) = 0;
};

class RmlUserInterfaceManager
{
public:
	virtual							~RmlUserInterfaceManager() = default;

	virtual void					Init() = 0;
	virtual void					Shutdown() = 0;
	virtual RmlUserInterface*		Find( const char* name, bool autoload ) = 0;
	virtual RmlUserInterface*		Find( const Rml::Context* context ) = 0;
	virtual void					Remove( RmlUserInterface* ui ) = 0;

	virtual void					BeginLevelLoad() = 0;
	virtual void					EndLevelLoad( const char* mapName ) = 0;
	virtual bool					InLevelLoad() const = 0;

	// Reloads changed guis, or all guis.
	virtual void					Preload( const char* mapName ) = 0;
	virtual void					Reload( bool all ) = 0;
	virtual	void					ReloadStyleSheets( bool all ) = 0;

	virtual void					PostRender() = 0;
};

extern RmlUserInterfaceManager* rmlManager;

#endif
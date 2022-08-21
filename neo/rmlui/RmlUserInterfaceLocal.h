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

#ifndef __RMLUSERINTERFACELOCAL_H__
#define __RMLUSERINTERFACELOCAL_H__

#include "rmlui/RmlUserInterface.h"

#include "RmlUi/Core.h"

#include "rmlui/D3RmlRender.h"
#include "rmlui/RmlFileSystem.h"
#include "rmlui/RmlSystem.h"
#include "rmlui/RmlFontEngine.h"

#include "ui/DeviceContext.h"

constexpr int kMaxDocuments = 128;

class RmlUserInterfaceLocal : public RmlUserInterface
{
public:

	RmlUserInterfaceLocal();

	~RmlUserInterfaceLocal() override;

	// Begin RmlUserInterface
	bool						Init( const char* name, idSoundWorld* soundWorld ) override;

	// Handles an event, can return an action string, the caller interprets
	// any return and acts accordingly
	const char*					HandleEvent( const sysEvent_t* event, int time ) override;

	// Handles a named event
	void						HandleNamedEvent( const char* eventName ) override;

	void						Reload() override;

	void						ReloadStyleSheet() override;

	// Repaints the ui
	void						Redraw( int time ) override;

	// Loads the document and sets up the event listeners.
	RmlDocHandle				LoadDocumentHandle( const char* filePath, RmlEventHandler* eventHandler = nullptr ) override;

	Rml::ElementDocument*		GetDocumentFromHandle( RmlDocHandle handle ) override;

	bool						IsDocumentOpen( const char* name ) override;

	void						CloseDocument( const char* name ) override;

	Rml::ElementDocument*		GetDocument( const char* _path ) override;


	void						SetCursor( float x, float y ) override
	{
		cursorX = x;
		cursorY = y;
	}

	float						CursorX() override
	{
		return cursorX;
	}

	float						CursorY() override
	{
		return cursorY;
	}

	bool						IsCursorEnabled() const override
	{
		return cursorEnabled;
	}

	void						SetCursorEnabled( bool newCursorEnabled ) override
	{
		cursorEnabled = newCursorEnabled;
	}

	// Activated the gui.
	const char*					Activate( bool activate ) override;

	Rml::ElementDocument*		SetNextScreen( const char* _nextScreen, RmlEventHandler* _eventHandler = nullptr ) override;

	void						HideAllDocuments() override;

	bool						IsActive() override
	{
		return isActive;
	}

	bool						IsPausingGame() override
	{
		return isPausingGame;
	}

	virtual void				SetIsPausingGame( bool pause ) override
	{
		isPausingGame = pause;
	}

	bool						InhibitsControl( ) override;

	idVec2						GetScreenSize() const override;

	void						SetSize( int width, int height ) override;

	void						SetUseScreenResolution( bool useScreen ) override;

	int							PlaySound( const char* sound, int channel = SCHANNEL_ANY, bool blocking = false ) override;

	void						StopSound( int channel = SCHANNEL_ANY ) override;

	// Adds a command to be picked up by the game logic.
	void						AddCommand( const char* _cmd ) override;

	// End RmlUserInterface

	size_t						Size();

	const char*					GetName() const
	{
		return name;
	}

	ID_TIME_T					GetTimeStamp() const
	{
		return timeStamp;
	}

	void						ClearRefs()
	{
		refs = 0;
	}

	void						AddRef()
	{
		refs++;
	}

	int							GetRefs()
	{
		return refs;
	}

	Rml::Context*				Context()
	{
		return context;
	}

	void						DrawCursor();

	void						SetInhibitsControl( bool inhibit )
	{
		inhibitsControl = inhibit;
	}

protected:


	/// @name Events
	/// @{
	void						HandleCharEvent( const sysEvent_t* event, int keyModState );

	void						HandleMouseWheelEvent( const sysEvent_t* event, int keyModState );

	void						HandleMouseButtonEvent( const sysEvent_t* event, int keyModState );

	void						HandleAbsoluteMouseEvent( const sysEvent_t* event, int keyModState );

	void						BoundCursorToScreen();

	void						HandleMouseEvent( const sysEvent_t* event, int keyModState );
	/// @}

	struct Document
	{
		Rml::ElementDocument*	doc = nullptr;			//!< @notowns
		RmlEventHandler*		eventHandler = nullptr; //!< @notowns
		ID_TIME_T				timeStamp = 0;			//!< @notowns
		idStr					name;					//!< @notowns
	};

	Document*					GetInternalDocument( const char* name );

	Rml::Context*					context;

	idStr							name;
	ID_TIME_T						timeStamp;

	bool							useScreenResolution;
	int								width;
	int								height;
	float							cursorX;
	float							cursorY;
	bool							cursorEnabled;
	bool							isActive;
	bool							isPausingGame;
	bool							inhibitsControl;

	int								refs;

	idSoundWorld*					soundWorld;

	idStr							cmds;
	idStr							pendingCmds;

	HandleManagerT<kMaxDocuments>	handleManager;
	idList<Document>				documents;
};

struct RmlImage
{
	idImage*			image = nullptr;
	const idMaterial*	material = nullptr;
	const byte*			data = nullptr;
	idVec2				dimensions = idVec2( 0.0f, 0.0f );
	bool				referencedOutsideLevelLoad = false;

	void Free();
};

class RmlUserInterfaceManagerLocal : public RmlUserInterfaceManager
{
public:

	virtual						~RmlUserInterfaceManagerLocal() = default;

	void						Init() override;
	void						Shutdown() override;
	RmlUserInterface*			Find( const char* name, bool autoload ) override;
	RmlUserInterface*			Find( const Rml::Context* context ) override;
	void						Remove( RmlUserInterface* gui ) override;

	void						BeginLevelLoad() override;
	void						EndLevelLoad( const char* mapName ) override;
	bool						InLevelLoad() const override
	{
		return inLevelLoad;
	}

	// Load all the materials.
	void						Preload( const char* mapName ) override;

	// Reloads changed guis, or all guis.
	void						Reload( bool all ) override;

	void						ReloadStyleSheets( bool all ) override;

	// Run this method on the main thread to actually generate data for the image. This is used to generate font glyphs to an idImage.
	void						PostRender() override;

private:

	/// Keep track of loaded documents and its timestamp to reload when files change.
	struct Document
	{
		Rml::ElementDocument*	doc = nullptr;
		ID_TIME_T				timeStamp = 0;
		idStr					name;
	};

	idDeviceContextOptimized		dc;

	idList<RmlUserInterfaceLocal*>	guis;
	idList<RmlImage>				imagesToReload;

	HandleManagerT<kMaxDocuments>	docHandleManager;
	idList<Document>				documents;
	idList<idStr>					docNames;

	idRmlSystem						rmlSystem;
	idRmlRender						rmlRender;
	RmlFileSystem					rmlFileSystem;
	RmlFontEngine					rmlFontEngine;
	bool							inLevelLoad;
};

#endif  // !__RMLUSERINTERFACE_H__
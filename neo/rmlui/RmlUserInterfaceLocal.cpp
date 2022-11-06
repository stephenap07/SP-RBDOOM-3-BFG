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
#include "RmlFileSystem.h"
#include "RmlEventHandler.h"
#include "GlobalRmlEventListener.h"

#include "RmlRenderDecorator.h"
#include "renderer/RenderCommon.h"

#include "RmlUi/Debugger.h"

RmlUserInterfaceManagerLocal rmlManagerLocal;
RmlUserInterfaceManager* rmlManager = &rmlManagerLocal;

idDeviceContext* rmlDc;

extern idCVar sys_lang;

class GlobalRmlEventListenerInstancer : public Rml::EventListenerInstancer
{
public:
	GlobalRmlEventListenerInstancer()
		: ui( nullptr )
		, eventHandler( nullptr )
	{
	}

	void SetUi( RmlUserInterface* _ui )
	{
		ui = _ui;
	}

	void SetEventHandler( RmlEventHandler* _eventHandler )
	{
		eventHandler = _eventHandler;
	}

	Rml::EventListener* InstanceEventListener( const Rml::String& value, Rml::Element* element ) override
	{
		auto listener = new GlobalRmlEventListener( ui, value, element );
		listener->SetEventHandler( eventHandler );
		return listener;
	}

private:

	RmlUserInterface* ui;
	RmlEventHandler* eventHandler;
};

static GlobalRmlEventListenerInstancer eventListenerInstancer;

static idRmlRenderDecoratorInstancer decoratorInstancer;

static constexpr int kMaxGuis = 64;

/*
===============
RmlUserInterfaceLocal

UI to represent a context. Handles events and contains rml file loading.
===============
*/
RmlUserInterfaceLocal::RmlUserInterfaceLocal()
	: context( nullptr )
	, timeStamp( 0 )
	, useScreenResolution( true )
	, width( SCREEN_WIDTH )
	, height( SCREEN_HEIGHT )
	, cursorX( 0 )
	, cursorY( 0 )
	, cursorEnabled( true )
	, isActive( false )
	, isPausingGame( false )
	, inhibitsControl( false )
	, refs( 1 )
	, soundWorld( nullptr )
{
}

RmlUserInterfaceLocal::~RmlUserInterfaceLocal()
{
	// NOTE(Stephen): Need to shutdown the debugger before freeing any documents. Seems like an oversight of 
	Rml::Debugger::Shutdown();
	handleManager.reset();
	documents.Clear();
	context->UnloadAllDocuments();
	Rml::RemoveContext( name.c_str() );
}

bool RmlUserInterfaceLocal::Init( const char* name, idSoundWorld* newSoundWorld )
{
	context = Rml::GetContext( name );
	soundWorld = newSoundWorld;
	name = name;
	cmds.Clear();

	if( !context )
	{
		const auto dim = GetScreenSize();
		context = Rml::CreateContext( name, Rml::Vector2i( dim.x, dim.y ) );
	}

	if( context )
	{
		context->EnableMouseCursor( true );
	}

	Rml::Debugger::Initialise( context );

	documents.SetNum( kMaxGuis );

	return context != nullptr;
}

const char* RmlUserInterfaceLocal::HandleEvent( const sysEvent_t* event, int time )
{
	if( !IsActive() )
	{
		return nullptr;
	}

	// Clear this command buffer out to populate them with new commands from the event handlers.
	cmds.Clear();

	const int keyModState = idRmlSystem::GetKeyModifier();

	if( event->IsMouseEvent() )
	{
		HandleMouseEvent( event, keyModState );
	}
	else if( event->IsMouseAbsoluteEvent() )
	{
		HandleAbsoluteMouseEvent( event, keyModState );
	}
	else if( event->evValue >= K_MOUSE1 && event->evValue <= K_MOUSE16 )
	{
		HandleMouseButtonEvent( event, keyModState );
	}
	else if( event->evValue == K_MWHEELDOWN || event->evValue == K_MWHEELUP )
	{
		HandleMouseWheelEvent( event, keyModState );
	}

	if( event->IsKeyEvent() )
	{
		const Rml::Input::KeyIdentifier key = idRmlSystem::TranslateKey( event->evValue );

		if( event->IsKeyDown() )
		{
			context->ProcessKeyDown( key, keyModState );
		}
		else if( event->IsKeyUp() )
		{
			context->ProcessKeyUp( key, keyModState );
		}
	}

	if( event->IsCharEvent() )
	{
		HandleCharEvent( event, keyModState );
	}

	if( cmds.Length() > 0 )
	{
		return cmds.c_str();
	}

	return nullptr;
}

void RmlUserInterfaceLocal::HandleNamedEvent( const char* eventName )
{
}

void RmlUserInterfaceLocal::HandleCharEvent( const sysEvent_t* event, int keyModState )
{
	Rml::Input::KeyIdentifier key = idRmlSystem::TranslateKey( event->evValue );

	context->ProcessKeyDown( key, keyModState );
}

void RmlUserInterfaceLocal::HandleMouseWheelEvent( const sysEvent_t* event, int keyModState )
{
	const float mouseWheel = ( event->evValue == K_MWHEELDOWN ) ? 1.0f : -1.0f;
	context->ProcessMouseWheel( mouseWheel, keyModState );
}

void RmlUserInterfaceLocal::HandleMouseButtonEvent( const sysEvent_t* event, int keyModState )
{
	int mouseButton;

	switch( event->evValue )
	{
		case K_MOUSE1:
			mouseButton = 0;
			break;
		case K_MOUSE2:
			mouseButton = 1;
			break;
		case K_MOUSE3:
			mouseButton = 2;
			break;
		case K_MOUSE4:
			mouseButton = 3;
			break;
		default:
			mouseButton = 0;
	}

	if( event->evValue2 )
	{
		context->ProcessMouseButtonDown( mouseButton, keyModState );
	}
	else
	{
		context->ProcessMouseButtonUp( mouseButton, keyModState );
	}
}

void RmlUserInterfaceLocal::HandleAbsoluteMouseEvent( const sysEvent_t* event, int keyModState )
{
	cursorX = event->evValue;
	cursorY = event->evValue2;

	const float pixelAspect = renderSystem->GetPixelAspect();
	const float sysWidth = renderSystem->GetWidth() * ( pixelAspect > 1.0f ? pixelAspect : 1.0f );
	const float sysHeight = renderSystem->GetHeight() / ( pixelAspect < 1.0f ? pixelAspect : 1.0f );
	const float scale = 1.0f * sysHeight / static_cast<float>( context->GetDimensions().y );
	const float invScale = 1.0f / scale;
	const float tx = 0.5f * ( sysWidth - ( context->GetDimensions().x * scale ) );
	const float ty = 0.5f * ( sysHeight - ( context->GetDimensions().y * scale ) );

	cursorX = idMath::Ftoi( ( static_cast<float>( event->evValue ) - tx ) * invScale );
	cursorY = idMath::Ftoi( ( static_cast<float>( event->evValue2 ) - ty ) * invScale );

	BoundCursorToScreen();

	context->ProcessMouseMove( cursorX, cursorY, keyModState );
}

void RmlUserInterfaceLocal::BoundCursorToScreen()
{
	if( cursorX < 0 )
	{
		cursorX = 0;
	}

	if( cursorY < 0 )
	{
		cursorY = 0;
	}

	if( ( cursorX + 1 ) > context->GetDimensions().x )
	{
		cursorX = context->GetDimensions().x - 1;
	}

	if( ( cursorY + 1 ) > context->GetDimensions().y )
	{
		cursorY = context->GetDimensions().y - 1;
	}
}

void RmlUserInterfaceLocal::HandleMouseEvent( const sysEvent_t* event, int keyModState )
{
	cursorX += event->evValue;
	cursorY += event->evValue2;

	BoundCursorToScreen();

	context->ProcessMouseMove( cursorX, cursorY, keyModState );
}

void RmlUserInterfaceLocal::Redraw( int time )
{
	if( !IsActive() )
	{
		return;
	}

	// Clear out any existing models.
	tr.guiModel->EmitFullScreen();

	const auto dim = GetScreenSize();
	context->SetDimensions( Rml::Vector2i( dim.x, dim.y ) );
	context->Update();
	const auto renderer = static_cast<idRmlRender*>( Rml::GetRenderInterface() );
	renderer->PreRender();

	// render the clip mask over the entire gui region first.
	renderer->EnableScissorRegion( true );
	renderer->SetScissorRegion( 0, 0, dim.x, dim.y );

	context->Render();
	DrawCursor();

	renderer->PostRender();
}

RmlDocHandle RmlUserInterfaceLocal::LoadDocumentHandle( const char* filePath, RmlEventHandler* eventHandler )
{
	if( !context )
	{
		return { kInvalidHandle };
	}

	// Set up the event listener to point to this document and the global event
	// handler.
	eventListenerInstancer.SetUi( this );
	eventListenerInstancer.SetEventHandler( eventHandler );

	// This touches the filesystem, not sure if this really should be doing this sort
	// of thing in the middle of the game. Need to preload as much as possible.
	ID_TIME_T timeStamp( 0 );
	fileSystem->ReadFile( filePath, nullptr, &timeStamp );

	Document* foundDoc = nullptr;

	RmlDocHandle handle;

	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		uint16_t id = handleManager.getHandleAt( i );
		if( context == documents[id].doc->GetContext() &&
				!idStr::Icmp( documents[id].name, filePath ) )
		{
			handle = { static_cast<uint16>( i ) };
			foundDoc = &documents[id];
			break;
		}
	}

	if( foundDoc )
	{
		if( timeStamp <= foundDoc->timeStamp )
		{
			// The already loaded document doesn't need an update.
			return handle;
		}
	}

	handle.id = handleManager.alloc();

	if( timeStamp != FILE_NOT_FOUND_TIMESTAMP )
	{
		Rml::ElementDocument* document = context->LoadDocument( filePath );
		if( document )
		{
			if( foundDoc )
			{
				foundDoc->doc->Close();
				foundDoc->doc = document;
				foundDoc->eventHandler = eventHandler;
				foundDoc->timeStamp = timeStamp;
			}
			else
			{
				documents[handle.id] = { document, eventHandler, timeStamp, filePath };
			}
		}
	}

	return handle;
}

Rml::ElementDocument* RmlUserInterfaceLocal::GetDocumentFromHandle( const RmlDocHandle handle )
{
	assert( handle.id != kInvalidHandle );
	return documents[handle.id].doc;
}

bool RmlUserInterfaceLocal::IsDocumentOpen( const char* name )
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		if( context == documents[id].doc->GetContext() && !idStr::Icmp( documents[id].name, name ) )
		{
			return true;
		}
	}

	return false;
}

bool RmlUserInterfaceLocal::IsDocumentOpen( RmlDocHandle handle ) const
{
	if( handleManager.isValid( handle.id ) )
	{
		uint16_t id = handleManager.getHandleAt( handle.id );
		if( context == documents[ id ].doc->GetContext() )
		{
			return true;
		}
	}

	return false;
}

void RmlUserInterfaceLocal::CloseDocument( const char* name )
{
	const int numHandles = handleManager.getNumHandles();
	for( int i = 0; i < numHandles; i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		if( context == documents[id].doc->GetContext() &&
				!idStr::Icmp( documents[id].name, name ) )
		{
			handleManager.free( id );
			documents[id].doc->Close();
			documents[id] = {};
			break;
		}
	}
}

Rml::ElementDocument* RmlUserInterfaceLocal::GetDocument( const char* name )
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		if( context == documents[id].doc->GetContext() &&
				!idStr::Icmp( documents[id].name, name ) )
		{
			return documents[i].doc;
		}
	}

	// didn't find the document
	return nullptr;
}

void RmlUserInterfaceLocal::Reload()
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		uint16_t id = handleManager.getHandleAt( i );
		auto& doc = documents[id];

		if( !doc.doc )
		{
			continue;
		}

		const bool isShown = doc.doc->IsVisible();

		ID_TIME_T timeStamp( 0 );
		fileSystem->ReadFile( doc.name.c_str(), nullptr, &timeStamp );
		if( timeStamp > doc.timeStamp )
		{
			// File needs a reload.
			common->DPrintf( "Reloading RML Doc %s\n", doc.name.c_str() );

			LoadDocumentHandle( doc.name, doc.eventHandler );

			if( isShown )
			{
				doc.doc->Show();
			}
		}
	}
}

void RmlUserInterfaceLocal::ReloadStyleSheet()
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		uint16_t id = handleManager.getHandleAt( i );
		auto& doc = documents[id];

		if( !doc.doc )
		{
			continue;
		}

		doc.doc->ReloadStyleSheet();
	}
}

const char* RmlUserInterfaceLocal::Activate( bool activate )
{
	if( !activate )
	{
		SetInhibitsControl( false );
	}

	isActive = activate;

	return nullptr;
}

Rml::ElementDocument* RmlUserInterfaceLocal::SetNextScreen( const char* _nextScreen, RmlEventHandler* _eventHandler )
{
	auto doc = LoadDocumentHandle( _nextScreen, _eventHandler );
	auto docPtr = GetDocumentFromHandle( doc );
	if( docPtr )
	{
		docPtr->Show();
	}
	return docPtr;
}

void RmlUserInterfaceLocal::HideAllDocuments()
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		uint16_t id = handleManager.getHandleAt( i );
		auto& doc = documents[id];
		if( doc.doc )
		{
			documents[id].doc->Hide();
		}
	}
}

void RmlUserInterfaceLocal::AddCommand( const char* _cmd )
{
	if( !cmds.IsEmpty() )
	{
		cmds.Append( ";" );
	}
	cmds.Append( _cmd );
}

size_t RmlUserInterfaceLocal::Size()
{
	return size_t();
}

void RmlUserInterfaceLocal::DrawCursor()
{
	if( !cursorEnabled )
	{
		return;
	}

	idRmlRender* renderer = static_cast<idRmlRender*>( Rml::GetRenderInterface() );
	renderer->DrawCursor( cursorX, cursorY, context->GetDimensions().x, context->GetDimensions().y );
}

bool RmlUserInterfaceLocal::InhibitsControl()
{
	if( !IsActive() )
	{
		return false;
	}

	return inhibitsControl;
}

idVec2 RmlUserInterfaceLocal::GetScreenSize() const
{
	if( useScreenResolution )
	{
		return idVec2( renderSystem->GetWidth(), renderSystem->GetHeight() );
	}

	return idVec2( width, height );
}

void RmlUserInterfaceLocal::SetSize( int newWidth, int newHeight )
{
	width = newWidth;
	height = newHeight;
}

void RmlUserInterfaceLocal::SetUseScreenResolution( bool useScreen )
{
	useScreenResolution = useScreen;
}

int RmlUserInterfaceLocal::PlaySound( const char* sound, int channel, bool blocking )
{
	if( !IsActive() )
	{
		return -1;
	}

	if( soundWorld )
	{
		return soundWorld->PlayShaderDirectly( sound, channel );
	}
	else
	{
		idLib::Warning( "No playing sound world on soundSystem in swf play sound!" );
		return -1;
	}
}

void RmlUserInterfaceLocal::StopSound( int channel )
{
	if( soundWorld )
	{
		soundWorld->PlayShaderDirectly( nullptr, channel );
	}
	else
	{
		idLib::Warning( "No playing sound world on soundSystem in rml play sound!" );
	}
}

RmlUserInterfaceLocal::Document* RmlUserInterfaceLocal::GetInternalDocument( const char* name )
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		uint16_t id = handleManager.getHandleAt( i );
		if( context == documents[id].doc->GetContext() &&
				!idStr::Icmp( documents[id].name, name ) )
		{
			return &documents[id];
		}
	}

	return nullptr;
}

/*
===============
RmlUserInterfaceManagerLocal

Manages a group of rml UI contexts and manages the lifetime of these objects through level loading.
===============
*/

void RmlUserInterfaceManagerLocal::Init()
{
	rmlRender.Init();

	Rml::SetRenderInterface( &rmlRender );
	Rml::SetSystemInterface( &rmlSystem );
	Rml::SetFileInterface( &rmlFileSystem );
	Rml::SetFontEngineInterface( &rmlFontEngine );

	if( !Rml::Initialise() )
	{
		common->Error( "Failed to initialize RML UI system" );
	}

	rmlFontEngine.Init();

	// Make this font face stuff configurable. Or put it in one of the base classes.
	struct FontFace
	{
		Rml::String filename;
		bool fallbackFace;
	};

	FontFace fontFaces[] =
	{
		{ "LatoLatin-Regular.ttf",    false },
		{ "LatoLatin-Italic.ttf",     false },
		{ "LatoLatin-Bold.ttf",       false },
		{ "LatoLatin-BoldItalic.ttf", false },
		{ "silent_hell/SLNTHLN.ttf",  false },
		{ "NotoEmoji-Regular.ttf",    true  },
	};

	for( const FontFace& face : fontFaces )
	{
		Rml::LoadFontFace( Rml::String( "newfonts/" ) + face.filename, face.fallbackFace );
	}

	// Register event handlers and decorators.
	Rml::Factory::RegisterEventListenerInstancer( &eventListenerInstancer );

	decoratorInstancer.Init();
	Rml::Factory::RegisterDecoratorInstancer( "render-decorator", &decoratorInstancer );

	dc.Init();

	rmlDc = &dc;

	inLevelLoad = false;
}

void RmlUserInterfaceManagerLocal::Shutdown()
{
	// The guis destructor is responsible for closing the documents within each gui context.
	guis.DeleteContents( true );
	Rml::Shutdown();
	rmlFontEngine.ReleaseFontResources();

}

RmlUserInterface* RmlUserInterfaceManagerLocal::Find( const char* name, bool autoload )
{
	int c = guis.Num();
	for( int i = 0; i < c; i++ )
	{
		if( !idStr::Icmp( guis[i]->GetName(), name ) )
		{
			if( guis[i]->GetRefs() == 0 )
			{
				guis[i]->Reload();
			}

			guis[i]->AddRef();

			return guis[i];
		}
	}

	RmlUserInterfaceLocal* ui = new RmlUserInterfaceLocal();

	guis.Append( ui );

	if( autoload )
	{
		// TODO(Stephen): This is reinitialized every time. Need a better way to declare guis and then create instances of them.
		ui->Init( name, common->MenuSW() );
	}

	return ui;
}

RmlUserInterface* RmlUserInterfaceManagerLocal::Find( const Rml::Context* context )
{
	for( int i = 0; i < guis.Num(); i++ )
	{
		if( guis[i]->Context() == context )
		{
			guis[i]->AddRef();

			return guis[i];
		}
	}

	return nullptr;
}

void RmlUserInterfaceManagerLocal::Remove( RmlUserInterface* gui )
{
	if( gui )
	{
		int c = guis.Num();
		for( int i = 0; i < c; i++ )
		{
			if( guis[i] == gui )
			{
				delete guis[i];
				guis.RemoveIndex( i );
				return;
			}
		}
	}
}

void RmlUserInterfaceManagerLocal::BeginLevelLoad()
{
	inLevelLoad = true;

	for( int i = 0; i < guis.Num(); i++ )
	{
		guis[i]->ClearRefs();
	}
}

void RmlUserInterfaceManagerLocal::EndLevelLoad( const char* mapName )
{
	inLevelLoad = false;

	int c = guis.Num();
	for( int i = 0; i < c; i++ )
	{
		if( guis[i]->GetRefs() == 0 )
		{
			delete guis[i];
			guis.RemoveIndex( i );
			i--;
			c--;
		}

		common->UpdateLevelLoadPacifier();
	}

	if( cvarSystem->GetCVarBool( "fs_buildresources" ) && mapName != NULL && mapName[0] != '\0' )
	{
		common->Printf( "TODO(Stephen): Implement generated RML binary gui" );
	}
}

void RmlUserInterfaceManagerLocal::Preload( const char* mapName )
{
	const Rml::StringList textureNames = Rml::GetTextureSourceList();

	for( const auto& texturePath : textureNames )
	{
		const idMaterial* material = declManager->FindMaterial( texturePath.c_str() );
	}
}

void RmlUserInterfaceManagerLocal::Reload( bool all )
{
	for( int i = 0; i < guis.Num(); i++ )
	{
		guis[i]->Reload();
		guis[i]->Context()->Update();
	}
}

void RmlUserInterfaceManagerLocal::ReloadStyleSheets( bool all )
{
	for( int i = 0; i < guis.Num(); i++ )
	{
		guis[i]->ReloadStyleSheet();
	}
}

void RmlUserInterfaceManagerLocal::PostRender()
{
}

CONSOLE_COMMAND( reloadRml, "Reload updated rml gui files", NULL )
{
	rmlManagerLocal.Reload( true );
}

CONSOLE_COMMAND( reloadRcss, "Reload updated RCSS", NULL )
{
	rmlManagerLocal.ReloadStyleSheets( true );
}

CONSOLE_COMMAND( rmlDebug, "Toggle rml debugger", NULL )
{
	Rml::Debugger::SetVisible( !Rml::Debugger::IsVisible() );
}
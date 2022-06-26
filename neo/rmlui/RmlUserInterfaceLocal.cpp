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
#include "d3xp/anim/Anim.h"
#include "sys/DeviceManager.h"

RmlUserInterfaceManagerLocal rmlManagerLocal;
RmlUserInterfaceManager* rmlManager = &rmlManagerLocal;

idDeviceContext* rmlDc;

extern idCVar sys_lang;

extern DeviceManager* deviceManager;

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

RmlUserInterfaceLocal::RmlUserInterfaceLocal( )
	: _context( nullptr )
	, _timeStamp( 0 )
	, _useScreenResolution( true )
	, _width( SCREEN_WIDTH )
	, _height( SCREEN_HEIGHT )
	, _cursorX( 0 )
	, _cursorY( 0 )
	, _cursorEnabled( true )
	, _isActive( false )
	, _isPausingGame( false )
	, _inhibitsControl( false )
	, _refs( 1 )
	, _soundWorld( nullptr )
{
}

RmlUserInterfaceLocal::~RmlUserInterfaceLocal( )
{
	handleManager.reset();
	_documents.Clear( );
	_context->UnloadAllDocuments( );
	Rml::RemoveContext( _name.c_str( ) );
}

bool RmlUserInterfaceLocal::Init( const char* name, idSoundWorld* soundWorld )
{
	_context = Rml::GetContext( name );
	_soundWorld = soundWorld;
	_name = name;
	_cmds.Clear( );

	if( !_context )
	{
		const auto dim = GetScreenSize( );
		_context = Rml::CreateContext( name, Rml::Vector2i( dim.x, dim.y ) );
	}

	if( _context )
	{
		_context->EnableMouseCursor( true );
	}

	_documents.SetNum( kMaxGuis );

	return _context != nullptr;
}

const char* RmlUserInterfaceLocal::HandleEvent( const sysEvent_t* event, int time )
{
	// Clear this command buffer out to populate them with new commands from the event handlers.
	_cmds.Clear( );

	const int keyModState = idRmlSystem::GetKeyModifier( );

	if( event->IsMouseEvent( ) )
	{
		HandleMouseEvent( event, keyModState );
	}
	else if( event->IsMouseAbsoluteEvent( ) )
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

		if( event->IsKeyDown( ) )
		{
			_context->ProcessKeyDown( key, keyModState );
		}
		else if( event->IsKeyUp( ) )
		{
			_context->ProcessKeyUp( key, keyModState );
		}
	}

	if( event->IsCharEvent( ) )
	{
		HandleCharEvent( event, keyModState );
	}

	if( _cmds.Length() > 0 )
	{
		return _cmds.c_str( );
	}

	return nullptr;
}

void RmlUserInterfaceLocal::HandleNamedEvent( const char* eventName )
{
}

void RmlUserInterfaceLocal::HandleCharEvent( const sysEvent_t* event, int keyModState )
{
	Rml::Input::KeyIdentifier key = idRmlSystem::TranslateKey( event->evValue );

	_context->ProcessKeyDown( key, keyModState );
}

void RmlUserInterfaceLocal::HandleMouseWheelEvent( const sysEvent_t* event, int keyModState )
{
	const float mouseWheel = ( event->evValue == K_MWHEELDOWN ) ? 1.0f : -1.0f;
	_context->ProcessMouseWheel( mouseWheel, keyModState );
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
		_context->ProcessMouseButtonDown( mouseButton, keyModState );
	}
	else
	{
		_context->ProcessMouseButtonUp( mouseButton, keyModState );
	}
}

void RmlUserInterfaceLocal::HandleAbsoluteMouseEvent( const sysEvent_t* event, int keyModState )
{
	_cursorX = event->evValue;
	_cursorY = event->evValue2;

	const float pixelAspect = renderSystem->GetPixelAspect();
	const float sysWidth = renderSystem->GetWidth() * ( pixelAspect > 1.0f ? pixelAspect : 1.0f );
	const float sysHeight = renderSystem->GetHeight() / ( pixelAspect < 1.0f ? pixelAspect : 1.0f );
	const float scale = 1.0f * sysHeight / static_cast<float>( _context->GetDimensions().y );
	const float invScale = 1.0f / scale;
	const float tx = 0.5f * ( sysWidth - ( _context->GetDimensions().x * scale ) );
	const float ty = 0.5f * ( sysHeight - ( _context->GetDimensions().y * scale ) );

	_cursorX = idMath::Ftoi( ( static_cast<float>( event->evValue ) - tx ) * invScale );
	_cursorY = idMath::Ftoi( ( static_cast<float>( event->evValue2 ) - ty ) * invScale );

	BoundCursorToScreen();

	_context->ProcessMouseMove( _cursorX, _cursorY, keyModState );
}

void RmlUserInterfaceLocal::BoundCursorToScreen()
{
	if( _cursorX < 0 )
	{
		_cursorX = 0;
	}

	if( _cursorY < 0 )
	{
		_cursorY = 0;
	}

	if( ( _cursorX + 1 ) > _context->GetDimensions().x )
	{
		_cursorX = _context->GetDimensions().x - 1;
	}

	if( ( _cursorY + 1 ) > _context->GetDimensions().y )
	{
		_cursorY = _context->GetDimensions().y - 1;
	}
}

void RmlUserInterfaceLocal::HandleMouseEvent( const sysEvent_t* event, int keyModState )
{
	_cursorX += event->evValue;
	_cursorY += event->evValue2;

	BoundCursorToScreen();

	_context->ProcessMouseMove( _cursorX, _cursorY, keyModState );
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
	_context->SetDimensions( Rml::Vector2i( dim.x, dim.y ) );
	_context->Update();
	const auto renderer = static_cast<idRmlRender*>( Rml::GetRenderInterface() );
	renderer->PreRender();

	// render the clip mask over the entire gui region first.
	renderer->EnableScissorRegion( true );
	renderer->SetScissorRegion( 0, 0, dim.x, dim.y );

	_context->Render();
	DrawCursor();

	renderer->PostRender();
}

Rml::ElementDocument* RmlUserInterfaceLocal::LoadDocument( const char* filePath, RmlEventHandler* eventHandler )
{
	if( !_context )
	{
		return nullptr;
	}

	// Set up the event listener to point to this document and the global event
	// handler.
	eventListenerInstancer.SetUi( this );
	eventListenerInstancer.SetEventHandler( eventHandler );

	// This touches the filesystem, not sure if this really should be doing this sort
	// of thing in the middle of the game. Need to preload as much as possible.
	ID_TIME_T timeStamp( 0 );
	fileSystem->ReadFile( filePath, nullptr, &timeStamp );

	//Document* foundDoc( GetInternalDocument( filePath ) );
	Document* foundDoc = nullptr;
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		if( _context == _documents[id]._doc->GetContext() &&
				!idStr::Icmp( _documents[id]._name, filePath ) )
		{
			foundDoc = &_documents[id];
		}
	}

	if( foundDoc && timeStamp <= foundDoc->_timeStamp )
	{
		// The already loaded document doesn't need an update.
		return foundDoc->_doc;
	}

	Rml::ElementDocument* document = nullptr;
	if( timeStamp != FILE_NOT_FOUND_TIMESTAMP )
	{
		document = _context->LoadDocument( filePath );

		if( document )
		{
			if( foundDoc )
			{
				foundDoc->_doc->Close();
				foundDoc->_doc = document;
				foundDoc->_eventHandler = eventHandler;
				foundDoc->_timeStamp = timeStamp;
			}
			else
			{
				const auto id = handleManager.alloc();
				_documents[id] = { document, eventHandler, timeStamp, filePath };
			}
		}
	}

	return document;
}

RmlDocHandle RmlUserInterfaceLocal::LoadDocumentHandle( const char* filePath, RmlEventHandler* eventHandler )
{
	if( !_context )
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
		const auto& id = handleManager.getHandleAt( i );
		if( _context == _documents[id]._doc->GetContext() &&
				!idStr::Icmp( _documents[id]._name, filePath ) )
		{
			handle = { static_cast<uint16>( i ) };
			foundDoc = &_documents[id];
			break;
		}
	}

	if( foundDoc )
	{
		if( timeStamp <= foundDoc->_timeStamp )
		{
			// The already loaded document doesn't need an update.
			return handle;
		}
	}

	handle.id = handleManager.alloc();

	if( timeStamp != FILE_NOT_FOUND_TIMESTAMP )
	{
		if( Rml::ElementDocument* document = _context->LoadDocument( filePath ) )
		{
			if( foundDoc )
			{
				foundDoc->_doc->Close();
				foundDoc->_doc = document;
				foundDoc->_eventHandler = eventHandler;
				foundDoc->_timeStamp = timeStamp;
			}
			else
			{
				_documents[handle.id] = { document, eventHandler, timeStamp, filePath };
			}
		}
	}

	return handle;
}

Rml::ElementDocument* RmlUserInterfaceLocal::GetDocumentFromHandle( const RmlDocHandle handle )
{
	assert( handle.id != kInvalidHandle );
	return _documents[handle.id]._doc;
}

bool RmlUserInterfaceLocal::IsDocumentOpen( const char* name )
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		if( _context == _documents[id]._doc->GetContext() && !idStr::Icmp( _documents[id]._name, name ) )
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
		if( _context == _documents[id]._doc->GetContext() &&
				!idStr::Icmp( _documents[id]._name, name ) )
		{
			handleManager.free( id );
			_documents[id]._doc->Close();
			_documents[id] = {};
			break;
		}
	}
}

Rml::ElementDocument* RmlUserInterfaceLocal::GetDocument( const char* name )
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		if( _context == _documents[id]._doc->GetContext() &&
				!idStr::Icmp( _documents[id]._name, name ) )
		{
			return _documents[i]._doc;
		}
	}

	// didn't find the document
	return nullptr;
}

void RmlUserInterfaceLocal::Reload()
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		auto& doc = _documents[id];

		const bool isShown = doc._doc->IsVisible();

		ID_TIME_T timeStamp( 0 );
		fileSystem->ReadFile( doc._name.c_str(), nullptr, &timeStamp );
		if( timeStamp > doc._timeStamp )
		{
			// File needs a reload.
			common->DPrintf( "Reloading RML Doc %s\n", doc._name.c_str() );

			LoadDocument( doc._name, doc._eventHandler );

			if( isShown )
			{
				doc._doc->Show();
			}
		}
	}
}

void RmlUserInterfaceLocal::ReloadStyleSheet( )
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		_documents[id]._doc->ReloadStyleSheet();
	}
}

const char* RmlUserInterfaceLocal::Activate( bool activate )
{
	if( !_isActive && activate )
	{
		SetInhibitsControl( false );
	}

	_isActive = activate;

	return nullptr;
}

Rml::ElementDocument* RmlUserInterfaceLocal::SetNextScreen( const char* _nextScreen, RmlEventHandler* _eventHandler )
{
	auto doc = LoadDocument( _nextScreen, _eventHandler );
	if( doc )
	{
		doc->Show();
	}
	return doc;
}

void RmlUserInterfaceLocal::HideAllDocuments()
{
	for( int i = 0; i < handleManager.getNumHandles(); i++ )
	{
		const auto id = handleManager.getHandleAt( i );
		_documents[id]._doc->Hide();
	}
}

void RmlUserInterfaceLocal::AddCommand( const char* _cmd )
{
	if( !_cmds.IsEmpty( ) )
	{
		_cmds.Append( ";" );
	}
	_cmds.Append( _cmd );
}

size_t RmlUserInterfaceLocal::Size()
{
	return size_t();
}

void RmlUserInterfaceLocal::DrawCursor()
{
	if( !_cursorEnabled )
	{
		return;
	}

	idRmlRender* renderer = static_cast<idRmlRender*>( Rml::GetRenderInterface() );
	renderer->DrawCursor( _cursorX, _cursorY, _context->GetDimensions().x, _context->GetDimensions().y );
}

bool RmlUserInterfaceLocal::InhibitsControl( )
{
	if( !IsActive( ) )
	{
		return false;
	}

	return _inhibitsControl;
}

idVec2 RmlUserInterfaceLocal::GetScreenSize() const
{
	if( _useScreenResolution )
	{
		return idVec2( renderSystem->GetWidth(), renderSystem->GetHeight() );
	}

	return idVec2( _width, _height );
}

void RmlUserInterfaceLocal::SetSize( int width, int height )
{
	_width = width;
	_height = height;
}

void RmlUserInterfaceLocal::SetUseScreenResolution( bool useScreen )
{
	_useScreenResolution = useScreen;
}

int RmlUserInterfaceLocal::PlaySound( const char* sound, int channel, bool blocking )
{
	if( !IsActive() )
	{
		return -1;
	}

	AddCommand( va( "play %s", sound ) );

	if( _soundWorld )
	{
		return _soundWorld->PlayShaderDirectly( sound, channel );
	}
	else
	{
		idLib::Warning( "No playing sound world on soundSystem in swf play sound!" );
		return -1;
	}
}

void RmlUserInterfaceLocal::StopSound( int channel )
{
	if( _soundWorld )
	{
		_soundWorld->PlayShaderDirectly( NULL, channel );
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
		const auto id = handleManager.getHandleAt( i );
		if( _context == _documents[id]._doc->GetContext() &&
				!idStr::Icmp( _documents[id]._name, name ) )
		{
			return &_documents[id];
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
	_rmlRender.Init();

	Rml::SetRenderInterface( &_rmlRender );
	Rml::SetSystemInterface( &_rmlSystem );
	Rml::SetFileInterface( &_rmlFileSystem );
	Rml::SetFontEngineInterface( &_rmlFontEngine );

	if( !Rml::Initialise() )
	{
		common->Error( "Failed to initialize RML UI system" );
	}

	_rmlFontEngine.Init();

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
		{ "NotoEmoji-Regular.ttf",    true  },
	};

	for( const FontFace& face : fontFaces )
	{
		Rml::LoadFontFace( Rml::String( "newfonts/" ) + face.filename, face.fallbackFace );
	}

	// Register event handlers and decorators.
	Rml::Factory::RegisterEventListenerInstancer( &eventListenerInstancer );

	decoratorInstancer.Init( );
	Rml::Factory::RegisterDecoratorInstancer( "render-decorator", &decoratorInstancer );

	_dc.Init();

	rmlDc = &_dc;

	_inLevelLoad = false;
}

void RmlUserInterfaceManagerLocal::Shutdown()
{
	// The guis destructor is responsible for closing the documents within each gui context.
	guis.DeleteContents( true );

	Rml::Shutdown();
	_rmlFontEngine.ReleaseFontResources();

}

RmlUserInterface* RmlUserInterfaceManagerLocal::Find( const char* name, bool autoload )
{
	int c = guis.Num();
	for( int i = 0; i < c; i++ )
	{
		if( !idStr::Icmp( guis[i]->GetName(), name ) )
		{
			if( guis[i]->GetRefs( ) == 0 )
			{
				guis[i]->Reload( );
			}

			guis[i]->AddRef( );

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
	_inLevelLoad = true;

	for( int i = 0; i < guis.Num( ); i++ )
	{
		guis[i]->ClearRefs( );
	}
}

void RmlUserInterfaceManagerLocal::EndLevelLoad( const char* mapName )
{
	_inLevelLoad = false;

	int c = guis.Num( );
	for( int i = 0; i < c; i++ )
	{
		if( guis[i]->GetRefs( ) == 0 )
		{
			delete guis[i];
			guis.RemoveIndex( i );
			i--;
			c--;
		}

		common->UpdateLevelLoadPacifier( );
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
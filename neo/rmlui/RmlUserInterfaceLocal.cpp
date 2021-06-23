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
#include "GlobalRmlEventListener.h"

#include "renderer/GLState.h"
#include "renderer/GuiModel.h"
#include "renderer/Image.h"


RmlUserInterfaceManagerLocal rmlManagerLocal;
RmlUserInterfaceManager* rmlManager = &rmlManagerLocal;

idDeviceContext* rmlDc;

extern idCVar sys_lang;

class GlobalRmlEventListenerInstancer : public Rml::EventListenerInstancer
{
public:
	GlobalRmlEventListenerInstancer()
		: ui( nullptr )
	{
	}

	void SetUi( RmlUserInterface* _ui )
	{
		ui = _ui;
	}

	Rml::EventListener* InstanceEventListener( const Rml::String& value, Rml::Element* element ) override
	{
		return new GlobalRmlEventListener( ui, value, element );
	}

private:

	RmlUserInterface* ui;
};

static GlobalRmlEventListenerInstancer eventListenerInstancer;


/*
===============
RmlUserInterfaceLocal

UI to represent a context. Handles events and contains rml file loading.
===============
*/

RmlUserInterfaceLocal::RmlUserInterfaceLocal()
	: _context( nullptr )
	, _name()
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
	, _soundWorld( nullptr )
	, _refs( 1 )
{
}

RmlUserInterfaceLocal::~RmlUserInterfaceLocal()
{
}

bool RmlUserInterfaceLocal::Init( const char* name, idSoundWorld* soundWorld )
{
	_context = Rml::GetContext( name );
	_soundWorld = soundWorld;
	_isActive = true;

	if( !_context )
	{
		const auto dim = GetScreenSize();
		_context = Rml::CreateContext( name, Rml::Vector2i( dim.x, dim.y ) );
	}

	if( _context )
	{
		_context->EnableMouseCursor( true );
	}

	return _context != nullptr;
}

const char* RmlUserInterfaceLocal::HandleEvent( const sysEvent_t* event, int time )
{
	int keyModState = idRmlSystem::GetKeyModifier();

	if( event->IsMouseEvent() )
	{
		HandleMouseEvent( event, keyModState );
	}
	else if( event->IsMouseAbsoluteEvent() )
	{
		HandleAbsoluteMouseEvent( event, keyModState );
	}

	if( event->evValue >= K_MOUSE1 && event->evValue <= K_MOUSE16 )
	{
		HandleMouseButtonEvent( event, keyModState );
	}

	if( event->evValue == K_MWHEELDOWN || event->evValue == K_MWHEELUP )
	{
		HandleMouseWheelEvent( event, keyModState );
	}

	if( event->IsCharEvent() )
	{
		HandleCharEvent( event, keyModState );
	}

	return nullptr;
}

void RmlUserInterfaceLocal::HandleCharEvent( const sysEvent_t* event, int keyModState )
{
	Rml::Input::KeyIdentifier key = idRmlSystem::TranslateKey( event->evValue );

	_context->ProcessKeyDown( key, keyModState );
}

void RmlUserInterfaceLocal::HandleMouseWheelEvent( const sysEvent_t* event, int keyModState )
{
	float mouseWheel = ( event->evValue == K_MWHEELDOWN ) ? 1.0f : -1.0f;
	_context->ProcessMouseWheel( mouseWheel, keyModState );
}

void RmlUserInterfaceLocal::HandleMouseButtonEvent( const sysEvent_t* event, int keyModState )
{
	int mouseButton = 0;

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
	float scale = 1.0f * sysHeight / ( float )_context->GetDimensions().y;
	float invScale = 1.0f / scale;
	float tx = 0.5f * ( sysWidth - ( _context->GetDimensions().x * scale ) );
	float ty = 0.5f * ( sysHeight - ( _context->GetDimensions().y * scale ) );

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

void RmlUserInterfaceLocal::HandleNamedEvent( const char* eventName )
{
}

void RmlUserInterfaceLocal::Redraw( int time )
{
	if( rmlManager->InLevelLoad() )
	{
		//return;
	}

	const auto dim = GetScreenSize();
	_context->SetDimensions( Rml::Vector2i( dim.x, dim.y ) );
	_context->Update();
	idRmlRender* renderer = ( ( idRmlRender* )Rml::GetRenderInterface() );
	renderer->PreRender();

	// render the clip mask over the entire gui region first.
	renderer->SetScissorRegion( 0, 0, dim.x, dim.y );
	renderer->EnableScissorRegion( true );
	renderer->RenderClipMask();

	_context->Render();
	DrawCursor();
}

Rml::ElementDocument* RmlUserInterfaceLocal::LoadDocument( const char* filePath )
{
	if( !_context )
	{
		return nullptr;
	}

	Rml::ElementDocument* foundDoc( GetDocument( filePath ) );
	if( foundDoc )
	{
		return foundDoc;
	}

	ID_TIME_T timeStamp( 0 );
	fileSystem->ReadFile( filePath, nullptr, &timeStamp );

	Rml::ElementDocument* document = nullptr;
	if( timeStamp != FILE_NOT_FOUND_TIMESTAMP )
	{
		eventListenerInstancer.SetUi( this );
		document = _context->LoadDocument( filePath );

		if( document )
		{
			_documents.Append( { document, timeStamp, filePath } );
		}
	}

	return document;
}

bool RmlUserInterfaceLocal::IsDocumentOpen( const char* name )
{
	for( int i = 0; i < _documents.Num(); i++ )
	{
		if( _context == _documents[i]._doc->GetContext() && !idStr::Icmp( _documents[i]._name, name ) )
		{
			return true;
		}
	}

	return false;
}

void RmlUserInterfaceLocal::CloseDocument( const char* name )
{
	idList<int> toRemove;
	for( int i = 0; i < _documents.Num(); i++ )
	{
		if( _context == _documents[i]._doc->GetContext() && !idStr::Icmp( _documents[i]._name, name ) )
		{
			_documents[i]._doc->Close();
			toRemove.Append( i );
		}
	}

	while( toRemove.Num() > 0 )
	{
		_documents.RemoveIndex( toRemove[0] );
		toRemove.RemoveIndex( 0 );
	}
}

void RmlUserInterfaceLocal::Reload()
{
	for( int i = 0; i < _documents.Num(); i++ )
	{
		ID_TIME_T timeStamp( 0 );
		fileSystem->ReadFile( _documents[i]._name.c_str(), nullptr, &timeStamp );
		if( timeStamp != _documents[i]._timeStamp )
		{
			// File needs a reload.
			common->Printf( "Reloading %s\n", _documents[i]._name.c_str() );
			bool show = _documents[i]._doc->IsVisible();
			Rml::Context* context = _documents[i]._doc->GetContext();
			context->UnloadDocument( _documents[i]._doc );
			eventListenerInstancer.SetUi( this );
			_documents[i]._doc = context->LoadDocument( _documents[i]._name.c_str() );
			if( show && _documents[i]._doc )
			{
				_documents[i]._doc->Show();
			}
		}
	}
}

const char* RmlUserInterfaceLocal::Activate( bool activate, int time )
{
	_isActive = activate;
	return nullptr;
}

Rml::ElementDocument* RmlUserInterfaceLocal::SetNextScreen( const char* _nextScreen )
{
	auto doc = LoadDocument( _nextScreen );
	if( doc )
	{
		doc->Show();
	}
	return doc;
}

void RmlUserInterfaceLocal::HideAllDocuments()
{
	for( int i = 0; i < _documents.Num(); i++ )
	{
		_documents[i]._doc->Hide();
	}
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

Rml::ElementDocument* RmlUserInterfaceLocal::GetDocument( const char* name )
{
	for( int i = 0; i < _documents.Num(); i++ )
	{
		if( _context == _documents[i]._doc->GetContext() && !idStr::Icmp( _documents[i]._name, name ) )
		{
			return _documents[i]._doc;
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

	if( !Rml::Initialise() )
	{
		common->Error( "Failed to initialize RML UI system" );
	}

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

	Rml::Factory::RegisterEventListenerInstancer( &eventListenerInstancer );

	_dc.Init();

	rmlDc = &_dc;

	_inLevelLoad = false;
}

void RmlUserInterfaceManagerLocal::Shutdown()
{
	int c = _guis.Num();
	for( int i = 0; i < c; i++ )
	{
		delete _guis[i];
	}

	_guis.Clear();
	Rml::Shutdown();
}

RmlUserInterface* RmlUserInterfaceManagerLocal::Find( const char* name, bool autoload )
{
	int c = _guis.Num();
	for( int i = 0; i < c; i++ )
	{
		if( !idStr::Icmp( _guis[i]->GetName(), name ) )
		{
			return _guis[i];
		}
	}

	RmlUserInterfaceLocal* ui = new RmlUserInterfaceLocal();

	_guis.Append( ui );

	if( autoload )
	{
		ui->Init( name, common->MenuSW() );
	}

	return ui;
}

RmlUserInterface* RmlUserInterfaceManagerLocal::Find( const Rml::Context* context )
{
	for( int i = 0; i < _guis.Num(); i++ )
	{
		if( _guis[i]->Context() == context )
		{
			return _guis[i];
		}
	}

	return nullptr;
}

void RmlUserInterfaceManagerLocal::BeginLevelLoad()
{
	_inLevelLoad = true;
}

void RmlUserInterfaceManagerLocal::EndLevelLoad( const char* mapName )
{
	_inLevelLoad = false;
}

void RmlUserInterfaceManagerLocal::Reload( bool all )
{
	for( int i = 0; i < _guis.Num(); i++ )
	{
		_guis[i]->Reload();
	}
}

void RmlUserInterfaceManagerLocal::PostRender()
{
	for( int i = 0; i < _imagesToReload.Num(); i++ )
	{
		RmlImage& img = _imagesToReload[i];
		img.image->GenerateImage(
			img.data,
			img.dimensions.x,
			img.dimensions.y,
			textureFilter_t::TF_NEAREST,
			textureRepeat_t::TR_CLAMP,
			textureUsage_t::TD_LOOKUP_TABLE_RGBA );
		if( img.referencedOutsideLevelLoad )
		{
			img.image->SetReferencedOutsideLevelLoad();
		}
		img.Free();
	}

	if( _imagesToReload.Num() == 0 )
	{
		return;
	}

	_imagesToReload.Clear();
}

void RmlUserInterfaceManagerLocal::AddMaterialToReload( RmlImage* rmlImage )
{
	_imagesToReload.Append( *rmlImage );
}

CONSOLE_COMMAND( reloadRml, "Reload updated rml gui files", NULL )
{
	rmlManagerLocal.Reload( true );
}
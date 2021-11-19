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

#include "RmlShell.h"

#include "../Game_local.h"

#include "rmlui/RmlUserInterfaceLocal.h"
#include "rmlui/GlobalRmlEventListener.h"
#include "rmlui/RmlEventHandler.h"

#include "RmlUi/Core/EventListener.h"
#include "RmlUi/Core/EventListenerInstancer.h"
#include "RmlUi/Core/TransformPrimitive.h"

// For some reason, rml relies on these container types. It uses value_type.

class UI_Shell;

class EventHandlerOptions : public RmlEventHandler
{
public:
	EventHandlerOptions( UI_Shell* _shell )
		: shell( _shell )
	{
	}

	virtual ~EventHandlerOptions() = default;

	void ProcessEvent( Rml::Event& _event, const Rml::String& _value ) override
	{
		if( _value == "restore" )
		{
			Rml::ElementDocument* optionsBody = _event.GetTargetElement()->GetOwnerDocument();

			if( !optionsBody )
			{
				return;
			}

			Rml::String windowModeId;
			switch( r_fullscreen.GetInteger() )
			{
				case 0: // windowed
					windowModeId = "windowed";
					break;
				case 1: // fullscreen
					windowModeId = "fullscreen";
					break;
				case 2: // windowed borderless
					windowModeId = "windowed_borderless";
					break;
			}

			Rml::ElementFormControlInput* windowModeOption = rmlui_dynamic_cast<Rml::ElementFormControlInput*>( optionsBody->GetElementById( windowModeId ) );

			if( windowModeOption )
			{
				windowModeOption->SetAttribute( "checked", "" );
			}

			Rml::ElementFormControlInput* accept = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "accept" ) );
			Rml::ElementFormControlInput* apply = rmlui_dynamic_cast< Rml::ElementFormControlInput* >( optionsBody->GetElementById( "apply" ) );

			if( accept )
			{
				accept->SetDisabled( true );
			}

			if( apply )
			{
				apply->SetDisabled( true );
			}
		}
		else if( _value == "store" )
		{
			const Rml::String subParm = _event.GetParameter<Rml::String>( "submit", "cancel" );
			if( subParm == "accept" || subParm == "apply" )
			{
				Rml::String windowMode = _event.GetParameter<Rml::String>( "window_mode", "fullscreen" );

				if( windowMode == "fullscreen" )
				{
					r_fullscreen.SetInteger( 1 );
				}
				else if( windowMode == "windowed" )
				{
					r_fullscreen.SetInteger( 0 );
				}
				else if( windowMode == "windowed_borderless" )
				{
					r_fullscreen.SetInteger( 2 );
				}

				int vidModeOption = _event.GetParameter<int>( "vid_mode", 0);

				r_vidMode.SetInteger( vidModeOption );

				cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "vid_restart\n" );
			}
			
			if( subParm == "cancel" || subParm == "accept" )
			{
				_event.GetTargetElement()->GetOwnerDocument()->Hide();
				shell->SetNextScreen( "startmenu" );
			}
		}
		else if( _value == "enable_accept" )
		{
			Rml::ElementDocument* optionsBody = _event.GetTargetElement()->GetOwnerDocument();

			if( optionsBody == nullptr )
			{
				return;
			}

			// Enable the accept button when values are changed
			Rml::ElementFormControlInput* accept = rmlui_dynamic_cast<Rml::ElementFormControlInput*>( optionsBody->GetElementById( "accept" ) );
			Rml::ElementFormControlInput* apply = rmlui_dynamic_cast<Rml::ElementFormControlInput*>( optionsBody->GetElementById( "apply" ) );

			if( accept )
			{
				accept->SetDisabled( false );
			}

			if( apply )
			{
				apply->SetDisabled( false );
			}
		}
	}

private:

	UI_Shell* shell;
};

static EventHandlerOptions* eventHandlerOptions = nullptr;

// UI Code

UI_Shell::UI_Shell()
	: _ui( nullptr )
{
	eventHandlerOptions = new EventHandlerOptions( this );
}

UI_Shell::~UI_Shell()
{
	delete eventHandlerOptions;
}

bool UI_Shell::Init( )
{
	_ui = rmlManager->Find( "shell", true );

	if( !_ui )
	{
		return false;
	}

	SetupDataBinding( );

	_ui->SetInhibitsControl( true );
	_ui->SetIsPausingGame( false );

	_ui->LoadDocument( "guis/rml/shell/options.rml", eventHandlerOptions );
	_ui->LoadDocument( "guis/rml/shell/startmenu.rml" );
	_ui->LoadDocument( "guis/rml/shell/loading.rml" );
	_ui->LoadDocument( "guis/rml/shell/game.rml" );
	_ui->LoadDocument( "guis/rml/shell/test.rml" );

	// Preload all the materials.
	rmlManager->Preload( "" );

	return SetNextScreen( "startmenu" );
}

struct ShellOptions
{
	std::vector<vidMode_t> vidModes;
	int windowMode;

	void Init( )
	{
		vidModes.clear( );
		idList<vidMode_t> tempModeList;
		R_GetModeListForDisplay( 0, tempModeList );
		for( int i = 0; i < tempModeList.Num( ); i++ )
		{
			vidModes.push_back( tempModeList[i] );
		}

		windowMode = r_fullscreen.GetInteger( );
	}

} shellOptions;

void UI_Shell::SetupDataBinding( )
{
	if( _ui->IsDocumentOpen( "guis/rml/shell/options.rml" ) )
	{
		// Already loaded the document. Don't set up data binding.
		return;
	}

	shellOptions.Init( );

	Rml::DataModelConstructor constructor = _ui->Context( )->CreateDataModel( "options" );

	if( !constructor )
	{
		return;
	}

	if( auto vidModeHandle = constructor.RegisterStruct<vidMode_t>( ) )
	{
		vidModeHandle.RegisterMember( "width", &vidMode_t::width );
		vidModeHandle.RegisterMember( "height", &vidMode_t::height );
		vidModeHandle.RegisterMember( "displayHz", &vidMode_t::displayHz );
	}

	constructor.RegisterArray<std::vector<vidMode_t>>( );
	constructor.Bind( "vidModes", &shellOptions.vidModes );

	vidModeModel = constructor.GetModelHandle( );
}

Rml::ElementDocument* UI_Shell::SetNextScreen( const char* name )
{
	idStr path( va( "guis/rml/shell/%s.rml", name ) );

	if( _currentScreen != name )
	{
		_previousScreen = _currentScreen;
		_currentScreen = name;
	}
	else
	{
		Rml::ElementDocument* doc = _ui->GetDocument( path.c_str() );

		if( doc )
		{
			if( !doc->IsVisible( ) )
			{
				doc->Show( );
			}

			return doc;
		}
	}

	RmlEventHandler* eventHandler( nullptr );

	Rml::ElementDocument*  document = _ui->SetNextScreen( path.c_str(), eventHandler );

	if( !document )
	{
		return nullptr;
	}

	if( !idStr::Icmp( name, "startmenu" ) )
	{
		// Initialize the start screen
		auto el = document->GetElementById( "start_game" );
		if( el )
		{
			auto p1 = Rml::Transform::MakeProperty( { Rml::Transforms::Rotate2D{10.f}, Rml::Transforms::TranslateX{100.f} } );
			auto p2 = Rml::Transform::MakeProperty( { Rml::Transforms::Scale2D{3.f} } );
			el->Animate( "transform", p1, 1.8f, Rml::Tween{ Rml::Tween::Elastic, Rml::Tween::InOut }, -1, true );
			el->AddAnimationKey( "transform", p2, 1.3f, Rml::Tween{ Rml::Tween::Elastic, Rml::Tween::InOut } );
		}
	}

	// When this object frees its resources after destructing, it frees itself using the overriden delete method.
	// Originally this was allocated with the non-overriden 'new' function. Annoying.
	Rml::StringList textureNames = Rml::GetTextureSourceList();

	for( const auto& texturePath : textureNames )
	{
		const idMaterial* material = declManager->FindMaterial( texturePath.c_str() );
		if( !material )
		{
			common->Warning( "Failed to load rml texture %s", texturePath.c_str() );
		}
	}

	return document;
}
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

#include "precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

#include "rmlui/RmlUserInterfaceLocal.h"
#include "rmlui/RmlEventHandler.h"
#include "rmlui/Core/Decorator.h"

#include "RmlShell.h"


/*
* Provides the size method to use for the standard c++ container type.
*/
template< typename T >
class RmlCompatibleIdList : public idList< T >
{
public:
	RmlCompatibleIdList(std::initializer_list<T> initializerList)
		: idList<T>(initializerList)
	{}

	RmlCompatibleIdList() : idList() {}

	int size() const
	{
		return Num();
	}
};

struct WindowSizePair
{
	int width, height;
};

inline bool operator==(const WindowSizePair& lhs, const WindowSizePair& rhs)
{
	return lhs.width == rhs.width && lhs.height == rhs.height;
}

static const char* positionNames[] = {
	"top",
	"center",
	"bottom",
	"left",
	"right"
};

struct Menu
{
	enum class Position : int {
		Top = 0,
		Center,
		Bottom,
		Left,
		Right
	};

	Rml::String name;
	Position position;
	Rml::String back;
	Rml::String logo;
	bool hidden = false;

	const char* getPosition() const
	{
		return positionNames[(int)position];
	}
};

struct Setting
{
	Rml::String key;
	Rml::String  typeOf;
	int category;
	Rml::String  name;
	int current;
	int min;
	int max;
	RmlCompatibleIdList<Rml::String> labels;
	int increment;
};

struct Category
{
	Rml::String name;
	int count = 0;
};

void UpdateSettingSelect(Setting& setting, Rml::ElementDocument* doc)
{
	Rml::ElementList elems;
	doc->QuerySelectorAll(elems, (Rml::String("[data-option='") + setting.key + "']").c_str());
	for( auto elem : elems )
	{
		Rml::ElementList values;
		elem->GetElementsByClassName( values, "value" );
		for( auto value : values )
		{
			value->SetInnerRML( setting.labels[setting.current].c_str() );
		}
	}
}

struct MenuData
{
	idHashTable<Menu> menus;
	Menu currentOptionMenu;
	RmlCompatibleIdList<Setting> settings;

	enum Categories {
		VIDEO = 0,
		CONTROLS,
		CATEGORIES_COUNT
	};

	RmlCompatibleIdList<Category> categories;

	MenuData()
	{
		using Position = Menu::Position;
		menus.Set( "main", Menu{ "main", Position::Top, "main2", "main" });
		menus.Set( "main2", Menu{ "main2", Position::Center, "", "main" });
		menus.Set( "main3", Menu{ "main3", Position::Top, "main", "main" });
		menus.Set( "credits", Menu{ "credits", Position::Top, "main2", "credits" });
		menus.Set( "options", Menu{ "options", Position::Center, "", "options" });

		// set up the settings options
		settings.Append(Setting{
			"fullscreen",
			"select", // typeOF
			VIDEO, // category
			"FULLSCREEN", // name
			1, // current option index (windowed)
			0, // min
			2, // max
			{
				"FULLSCREEN",
				"WINDOWED",
				"WINDOWED BORDERLESS"
			},
			1
		});

		settings.Append(Setting{
			"quality",
			"select",
			VIDEO,
			"QUALITY PRESET",
			1,
			0,
			2,
			{
				"LOW",
				"MEDIUM",
				"HIGH"
			},
			1
		});

		settings.Append(Setting{
			"keypad",
			"select",
			CONTROLS,
			"CONTROLS METHOD",
			1,
			0,
			1,
			{
				"GAMEPAD",
				"KEYBOARD"
			},
			1
		});

		settings.Append(Setting{
			"infsens",
			"select",
			CONTROLS,
			"INFANTRY SENSITIVITY",
			1,
			0,
			100,
			{},
			5
		});

		categories.SetNum( CATEGORIES_COUNT );
		categories[VIDEO] = { "video" };
		categories[CONTROLS] = { "controls" };
	}

	void UpdateModel( Rml::Context& context )
	{
		Rml::DataModelConstructor constructor = context.CreateDataModel( "menudata" );

		if( !constructor )
		{
			return;
		}

		constructor.RegisterArray<RmlCompatibleIdList<Rml::String>>();

		// Register the types first
		if( auto settingsHandle = constructor.RegisterStruct<Setting>() )
		{
			settingsHandle.RegisterMember( "key", &Setting::key );
			settingsHandle.RegisterMember( "typeOf", &Setting::typeOf );
			settingsHandle.RegisterMember( "category", &Setting::category );
			settingsHandle.RegisterMember( "name", &Setting::name );
			settingsHandle.RegisterMember( "current", &Setting::current );
			settingsHandle.RegisterMember( "min", &Setting::min );
			settingsHandle.RegisterMember( "max", &Setting::max );
			settingsHandle.RegisterMember( "labels", &Setting::labels );
			settingsHandle.RegisterMember( "increment", &Setting::increment );
		}

		constructor.RegisterArray<RmlCompatibleIdList<Setting>>();

		if( auto categoryHandle = constructor.RegisterStruct<Category>() )
		{
			categoryHandle.RegisterMember( "name", &Category::name );
			categoryHandle.RegisterMember( "count", &Category::count );
		}

		constructor.RegisterArray<RmlCompatibleIdList<Category>>();

		// Bind variables to the data model
		constructor.Bind( "settings", &settings );
		constructor.Bind( "categories", &categories );
	}
};

static MenuData menuData;

class StartMenu_ParallaxEffect_Listener : public Rml::EventListener
{
public:
	StartMenu_ParallaxEffect_Listener(const char* elemName, Rml::ElementDocument* doc)
	{
		elem = doc->GetElementById(elemName);
	}

	void ProcessEvent(Rml::Event& event) override
	{
		if (!elem)
		{
			return;
		}

		float _w = event.GetCurrentElement()->GetClientWidth() / 2;
		float _h = event.GetCurrentElement()->GetClientHeight() / 2;
		auto mouseVec = event.GetUnprojectedMouseScreenPos();
		float _mouseX = mouseVec.x;
		float _mouseY = mouseVec.y;

		float ratio = 0.01f;

		int numChildren = elem->GetNumChildren();
		for (int i = 0; i < numChildren; i++)
		{
			Rml::Element* childElem = elem->GetChild(i);

			float depthX = (_mouseX - _w) * ratio;
			float depthY = (_mouseY - _h) * ratio;

			if (childElem)
			{
				//childElem->SetProperty("transform", va("translate(%.2f%%, %.2f%%)", depthX, depthY));
				auto p = Rml::Transform::MakeProperty({ Rml::Transforms::TranslateX{ depthX, Rml::Property::PX }, Rml::Transforms::TranslateY{ depthY, Rml::Property::PX } });
				childElem->SetProperty(Rml::PropertyId::Transform, p);
			}
			
			ratio += 0.01f;
		}
	}

private:

	Rml::Element* elem;
};

static void ReadMenuAttributes(const char* dataMenuStr, int length, idStr& from, idStr& to, idStr& transition)
{
	idLexer src(dataMenuStr, length, "menuattr");
	idToken token;

	while (!src.PeekTokenString(","))
	{
		src.ReadToken(&token);
		from.Append(token);
	}
	src.ExpectTokenString(",");

	while (!src.EndOfFile() && !src.PeekTokenString(","))
	{
		src.ReadToken(&token);
		to.Append(token);
	}

	if (src.PeekTokenString(","))
	{
		src.ExpectTokenString(",");
	}

	while (!src.EndOfFile() && !src.PeekTokenString(","))
	{
		src.ReadToken(&token);
		transition.Append(token);
	}
}


class StartMenu_MenuListener : public Rml::EventListener
{
public:
	explicit StartMenu_MenuListener(UI_Shell* inShell)
		: shell(inShell)
	{
	}

	void ProcessEvent( Rml::Event& event ) override
	{
		auto target = event.GetTargetElement();
		Rml::Variant* dataMenu = target->GetAttribute( "data-menu" );
		if( dataMenu )
		{
			shell->TransitionMenu(dataMenu->Get<Rml::String>().c_str());
		}
	}

private:
	UI_Shell* shell;
};

class OptionMenuListener : public Rml::EventListener
{
public:
	explicit OptionMenuListener(UI_Shell* inShell)
		: shell(inShell)
	{
	}

	void ProcessEvent(Rml::Event& event) override
	{
		auto target = event.GetTargetElement();
		Rml::Variant* dataMenu = target->GetAttribute("data-menu");
		if (dataMenu)
		{
			shell->TransitionMenuOption(dataMenu->Get<Rml::String>().c_str(), false);
		}
	}

private:
	UI_Shell* shell;
};

class OptionSelectListener : public Rml::EventListener
{
public:
	explicit OptionSelectListener(UI_Shell* inShell)
		: shell(inShell)
	{
	}

	void ProcessEvent(Rml::Event& event) override
	{
		auto setting = event.GetTargetElement()->GetAttribute("data-setting");
		if( setting )
		{
			shell->ChangeSettingsMenu( setting->Get<Rml::String>().c_str() );
		}
	}

private:
	UI_Shell* shell;
};

class StartMenu_BackButtonListener : public Rml::EventListener
{
public:
	explicit StartMenu_BackButtonListener(UI_Shell* controller)
		: shell(controller)
	{
	}

	void ProcessEvent(Rml::Event& event) override
	{
		shell->TransitionMenu(event.GetTargetElement()->GetAttribute("data-action")->Get<Rml::String>().c_str());
	}

private:
	UI_Shell* shell;
};

class BackButtonOptionListener : public Rml::EventListener
{
public:
	explicit BackButtonOptionListener(UI_Shell* controller)
		: shell(controller)
	{
	}

	void ProcessEvent(Rml::Event& event) override
	{
		shell->TransitionMenuOption(event.GetTargetElement()->GetAttribute("data-action")->Get<Rml::String>().c_str(), true);
	}

private:
	UI_Shell* shell;
};

class StartMenu_TransitionEndHideListener : public Rml::EventListener
{
public:
	void ProcessEvent(Rml::Event& event) override
	{
		auto target = event.GetTargetElement();
		if (target->IsClassSet("fadeout"))
		{
			target->SetClass("fadeout", false);
			target->SetClass("fadein", false);
			target->SetProperty(Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Hidden));
			target->SetProperty(Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::None));
		}
		if (target->IsClassSet("fadein"))
		{
			target->SetClass("fadeout", false);
			target->SetClass("fadein", false);
		}
	}
};

class MouseOverButtonListener : public Rml::EventListener
{
public:
	MouseOverButtonListener(RmlUserInterface* inUi) : ui(inUi) {}

	void ProcessEvent(Rml::Event& event) override
	{
		auto current = event.GetCurrentElement();
		if (current->IsClassSet("hidden") || !current->IsVisible())
		{
			return;
		}
		auto target = event.GetTargetElement();
		auto propertySound = target->GetAttribute( "data-sound" );
		if (propertySound)
		{
			auto soundFile = propertySound->Get<Rml::String>();
			ui->PlaySound( soundFile.c_str() );
		}
	}

private:
	RmlUserInterface* ui;
	idStr soundFile;
};

// For some reason, rml relies on these container types. It uses value_type.

class UI_Shell;

// UI Code

UI_Shell::UI_Shell()
	: ui( nullptr )
	, soundWorld( nullptr )
	, nextState( ShellState::WAITING )
	, state( ShellState::WAITING )
	, activeDoc()
	, nextDoc()
	, isInitialized( false )
	, gameComplete( false )
	, inGame( false )
	, eventHandlerOptions( this )
	, baseEventHandler( this )
	, startMenuParallaxEffectListener( nullptr )
{
}

UI_Shell::~UI_Shell()
{
	rmlManager->Remove( ui );
	// It's possible that the documents/context isn't actually cleared. Make sure to keep the listener around in that case?
	//delete startMenuParallaxEffectListener;
}

bool UI_Shell::Init( const char* filename,  idSoundWorld* sw )
{
	ui = rmlManager->Find( "shell", false );

	docStack.AssureSize( 4 );

	if( !ui )
	{
		return false;
	}

	ui->Init( "shell", sw );

	isInitialized = true;

	soundWorld = sw;

	SetupDataBinding( );

	ui->SetInhibitsControl( true );
	ui->SetIsPausingGame( false );

	// Load up all the documents.
	optionsDoc = ui->LoadDocumentHandle( "guis/rml/shell/options.rml", &eventHandlerOptions );
	startDoc = ui->LoadDocumentHandle( "guis/rml/shell/doom3start.rml", &baseEventHandler );
	loadingDoc = ui->LoadDocumentHandle( "guis/rml/shell/loading.rml", &baseEventHandler );
	pauseDoc = ui->LoadDocumentHandle( "guis/rml/shell/pause.rml", &baseEventHandler );

	Rml::ElementDocument* doc = ui->GetDocumentFromHandle( startDoc );
	auto startMenuListener = new StartMenu_MenuListener( this );
	auto mouseOverListener = new MouseOverButtonListener( ui );
	auto optionMenuListener = new OptionMenuListener( this );
	if( doc )
	{
		doc->GetElementById("menu")->AddEventListener(Rml::EventId::Animationend, new StartMenu_TransitionEndHideListener());
		startMenuParallaxEffectListener = new StartMenu_ParallaxEffect_Listener( "parallax", doc );
		doc->AddEventListener( Rml::EventId::Mousemove, startMenuParallaxEffectListener );
		for (int i = 0; i < menuData.menus.Num(); i++)
		{
			Menu* menu = menuData.menus.GetIndex(i);
			auto menuElem = doc->GetElementById(menu->name.c_str());
			if (menuElem)
			{
				menuElem->AddEventListener(Rml::EventId::Click, startMenuListener);
				menuElem->AddEventListener(Rml::EventId::Mouseover, mouseOverListener);
				idStr newClass = "menu-pos-";
				newClass.Append(menu->getPosition());
				menuElem->SetClass(newClass.c_str(), true);
			}
		}
		doc->GetElementById("back")->AddEventListener(Rml::EventId::Click, new StartMenu_BackButtonListener(this));

		Rml::ElementList menuOptionElems;
		//doc->QuerySelectorAll(menuOptionElems, "[data-action=\"menu-option\"]");
		doc->GetElementsByClassName(menuOptionElems, "menu-option");
		for (auto elem : menuOptionElems)
		{
			elem->AddEventListener(Rml::EventId::Click, optionMenuListener);
		}

		doc->GetElementById("back-options")->AddEventListener(Rml::EventId::Click, new BackButtonOptionListener(this));

		Rml::ElementList optionList;
		doc->QuerySelectorAll(optionList, ".options-select .selection");
		for (auto elem : optionList)
		{
			elem->AddEventListener(Rml::EventId::Click, new OptionSelectListener(this));
		}
	}

	// Preload all the materials.
	rmlManager->Preload( "" );

	prevFadeTime = Sys_Milliseconds() / 1000.f;

	animationTime = 400; // milliseconds

	currentMenu = "menu2";

	currentSetting = "menu";

	return true;
}


class ShellOptions
{
public:

	void Init()
	{
		vidModes.Clear();
		R_GetModeListForDisplay( 0, vidModes );
		for( int i = 0; i < vidModes.Num(); i++ )
		{
			windowSizes.AddUnique( { vidModes[i].width, vidModes[i].height } );
			displayHzs.AddUnique( vidModes[i].displayHz );
		}

		windowMode = r_fullscreen.GetInteger();
		FindLocalIndexes( windowSize, displayHz );
	}

	int FindVidModeIndex( const int windowSizeIndex, const int displayIndex ) const
	{
		const int width = windowSizes[windowSizeIndex].width;
		const int height = windowSizes[windowSizeIndex].height;
		const int di = displayHzs[displayIndex];

		for( int i = 0; i < vidModes.Num(); i++ )
		{
			if( vidModes[i].width == width && vidModes[i].height == height && vidModes[i].displayHz == di )
			{
				return i;
			}
		}

		return -1;
	}

	void FindLocalIndexes( int& windowSizeIndex, int& displayIndex ) const
	{
		const int vidModeNum = r_vidMode.GetInteger();

		if( vidModeNum > vidModes.Num() || vidModeNum < 0 )
		{
			common->FatalError( "Invalid vidMode set %d", vidModeNum );
			return;
		}

		const auto vidMode = vidModes[vidModeNum];
		for( int i = 0; i < windowSizes.Num(); i++ )
		{
			if( vidMode.width == windowSizes[i].width &&
					vidMode.height == windowSizes[i].height )
			{
				windowSizeIndex = i;
				break;
			}
		}

		for( int i = 0; i < displayHzs.Num(); i++ )
		{
			if( vidMode.displayHz == displayHzs[i] )
			{
				displayIndex = i;
				break;
			}
		}
	}

	RmlCompatibleIdList<vidMode_t>		vidModes;		//!< Available video modes queried from the system.
	RmlCompatibleIdList<WindowSizePair>	windowSizes;	//!< Locally indexed available window sizes.
	RmlCompatibleIdList<int>			displayHzs;		//!< Locally indexed available display hz.

	int									windowMode;		//!< The current selected video mode (fullscreen, windowed, windowed borderless, etc..).
	int									displayHz;		//!< The current selected display refresh rate.
	int									windowSize;		//!< The current selected window size.

	Rml::DataModelHandle				modelHandle;
};

ShellOptions shellOptions;

void UI_Shell::Update()
{
	HandleStateChange();

	HandleScreenChange();

	ui->Redraw( Sys_Milliseconds() / 1000.0f );
}

void UI_Shell::HandleStateChange( )
{
	// State Machine
	if( nextState != state )
	{
		nextDoc.id = kInvalidHandle;

		if( state == ShellState::START )
		{
			nextDoc = startDoc;
		}

		if( nextState == ShellState::PAUSED )
		{
			if( gameComplete )
			{
				// show credits
				nextState = ShellState::CREDITS;
			}
			else
			{
				nextDoc = pauseDoc;
				state = nextState;
			}
		}

		if( nextState == ShellState::START )
		{
			nextDoc = startDoc;
			state = nextState;
		}
		else if( nextState == ShellState::GAME )
		{
			if( gameComplete )
			{
				nextState = ShellState::CREDITS;
			}
			else
			{
				state = nextState;
			}
		}
		else if( nextState == ShellState::LOADING )
		{
			nextDoc = loadingDoc;
			state = nextState;
		}
	}
}


void UI_Shell::HandleScreenChange()
{
	if( activeDoc == nextDoc )
	{
		return;
	}

	if( activeDoc.id != kInvalidHandle )
	{
		ShowScreen( activeDoc, false );
	}

	if( nextDoc.id != kInvalidHandle )
	{
		ShowScreen( nextDoc, true );
	}

	activeDoc = nextDoc;
}

void UI_Shell::SetState( ShellState _nextState )
{
	nextState = _nextState;
}

void UI_Shell::SetupDataBinding()
{
	shellOptions.Init();

	ui->Context()->RemoveDataModel( "options" );

	Rml::DataModelConstructor constructor = ui->Context()->CreateDataModel( "options" );

	if( !constructor )
	{
		return;
	}

	// Register the types first
	if( auto vidModeHandle = constructor.RegisterStruct<WindowSizePair>() )
	{
		vidModeHandle.RegisterMember( "width", &WindowSizePair::width );
		vidModeHandle.RegisterMember( "height", &WindowSizePair::height );
	}
	constructor.RegisterArray<RmlCompatibleIdList<WindowSizePair>>();
	constructor.RegisterArray<RmlCompatibleIdList<int>>();

	// Bind variables to the data model
	constructor.Bind( "currentWindowSize", &shellOptions.windowSize );
	constructor.Bind( "currentDisplayHz", &shellOptions.displayHz );
	constructor.Bind( "windowSizes", &shellOptions.windowSizes );
	constructor.Bind( "displayHzs", &shellOptions.displayHzs );

	shellOptions.modelHandle = constructor.GetModelHandle();

	menuData.UpdateModel(*ui->Context());
}

void UI_Shell::ActivateMenu( const bool show )
{
	if( !ui )
	{
		return;
	}

	if( show && ui->IsActive() )
	{
		return;
	}

	if( !show && !ui->IsActive() )
	{
		return;
	}

	if( inGame )
	{
		if( const idPlayer* player = gameLocal.GetLocalPlayer() )
		{
			if( !show )
			{
				if( player->IsDead() && !common->IsMultiplayer() )
				{
					return;
				}
			}
		}
	}

	ui->Activate( show );
	
	if( show )
	{
		//if( !inGame )
		//{
		//	common->MenuSW()->PlaySound( GUI_SOUND_MUSIC );
		//}
	}
	else
	{
		nextDoc = startDoc;
		activeDoc = startDoc;
		nextState = ShellState::START;
		state = ShellState::START;
		common->Dialog().ClearDialog( GDM_LEAVE_LOBBY_RET_NEW_PARTY );
	}
}

void UI_Shell::SetNextScreen( const char* nextScreen )
{
	if( !nextScreen )
	{
		ui->Activate( false );
		ui->SetIsPausingGame( false );
		return;
	}

	if( idStr::Icmp( "options", nextScreen ) == 0 )
	{
		nextDoc = optionsDoc;
	}
	else if( idStr::Icmp( "startmenu", nextScreen ) == 0 )
	{
		nextDoc = startDoc;
	}
	else if( idStr::Icmp( "loading", nextScreen ) == 0 )
	{
		nextDoc = loadingDoc;
	}
	else if( idStr::Icmp( "pause", nextScreen ) == 0 )
	{
		nextDoc = pauseDoc;
	}
	else
	{
		common->Warning( "Setting the next screen in the shell to an invalid screen %s", nextScreen );
		ui->Activate( false );
	}
}

void UI_Shell::ShowScreen( RmlDocHandle handle, bool show )
{
	auto docPtr = ui->GetDocumentFromHandle( handle );
	if( docPtr )
	{
		show ? docPtr->Show() : docPtr->Hide();
	}
}

void UI_Shell::UpdateSavedGames()
{
}

int UI_Shell::FindVidModeIndex( int windowSizeIndex, int displayIndex ) const
{
	return shellOptions.FindVidModeIndex( windowSizeIndex, displayIndex );
}

bool UI_Shell::IsPausingGame()
{
	return ui->IsPausingGame();
}

void UI_Shell::TransitionMenu( const idStr& dataMenuStr )
{
	idStr to, from, transition;
	ReadMenuAttributes(dataMenuStr.c_str(), dataMenuStr.Length(), from, to, transition);

	auto current = ui->GetDocumentFromHandle(startDoc)->GetElementById(from.c_str());
	if (!current)
	{
		return;
	}

	auto nextElem = current->GetOwnerDocument()->GetElementById( to.c_str() );

	if( !nextElem )
	{
		return;
	}

	Menu* fromMenu;
	if( menuData.menus.Get(from.c_str(), &fromMenu) )
	{
	}

	Menu* toMenu;
	if( menuData.menus.Get(to.c_str(), &toMenu) )
	{
		if (transition == "vertical")
		{
			Menu::Position fromPosition = (toMenu->position == Menu::Position::Top) ? Menu::Position::Bottom : Menu::Position::Top;
			fromMenu->position = fromPosition;
			toMenu->position = Menu::Position::Center;
		}
		else if (transition == "horizontal")
		{
			Menu::Position fromPosition = (toMenu->position == Menu::Position::Left) ? Menu::Position::Right : Menu::Position::Left;
			fromMenu->position = fromPosition;
			toMenu->position = Menu::Position::Center;
		}
		else if (transition == "fade")
		{
			fromMenu->hidden = true;
			toMenu->hidden = false;
		}

		if (!toMenu->logo.empty())
		{
			idStr classname("animated ");
			classname.Append( toMenu->logo.c_str() );
			auto logoElem = current->GetOwnerDocument()->GetElementById("logo");
			if (logoElem)
			{
				logoElem->SetClassNames(classname.c_str());
			}
		}
		else
		{
			auto logoElem = current->GetOwnerDocument()->GetElementById("logo");
			if (logoElem)
			{
				logoElem->SetClassNames("animated hidden");
			}
		}

		if (!toMenu->back.empty())
		{
			if (from == "options")
			{
				transition = "vertical";
			}
			auto backElem = current->GetOwnerDocument()->GetElementById("back");
			if (backElem)
			{
				backElem->SetClass("fadein", 1);
				backElem->SetProperty(Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Visible));
				backElem->SetProperty(Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::Block));
				idStr attr(to.c_str());
				attr.Append(",");
				attr.Append(toMenu->back.c_str());
				attr.Append(",");
				attr.Append(transition);
				backElem->SetAttribute("data-action", attr.c_str());
			}
		}
		else
		{
			auto backElem = current->GetOwnerDocument()->GetElementById("back");
			if (backElem)
			{
				backElem->SetClass("fadeout", true);
			}
		}

		idStr newClass = (from == "options") ? "options-menu menu-pos-"  : "menu-container animated menu-pos-";
		newClass.Append(fromMenu->getPosition());

		idStr newClass2 = (to == "options") ? "options-menu menu-pos-" : "menu-container animated menu-pos-";
		newClass2.Append(toMenu->getPosition());

		if( transition == "fade" )
		{
			newClass.Append( fromMenu->hidden ? " fadeout" : " fadein" );
			newClass2.Append( toMenu->hidden ? " fadeout" : " fadein" );
		}

		current->SetClassNames(newClass.c_str());
		nextElem->SetClassNames(newClass2.c_str());

		// TODO: When moving into the options menu, it should show the specific options section 
	}
	else if(from == "dewrito-options")
	{
		// option menu
		menuData.currentOptionMenu = Menu{ to, Menu::Position::Center, "", "" };
	}

	nextElem->SetProperty(Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Visible));
	nextElem->SetProperty(Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::Block));
}

void UI_Shell::TransitionMenuOption(const idStr& command, bool useOptionBackButton)
{
	auto doc = ui->GetDocumentFromHandle(startDoc);
	if (!doc)
	{
		return;
	}

	idStr to, from, transition;
	ReadMenuAttributes(command.c_str(), command.Length(), from, to, transition);

	auto toElem = doc->GetElementById(to.c_str());
	if (!toElem)
	{
		return;
	}
	auto fromElem = doc->GetElementById(from.c_str());
	if (!fromElem)
	{
		return;
	}

	Rml::Variant* menuBack = fromElem->GetAttribute("data-menu-back");

	if (menuBack && menuBack->Get<Rml::String>() == "main2" && useOptionBackButton)
	{
		TransitionMenu("options,main2,fade");
	}
	else
	{
		fromElem->SetClass("fadeout", true);
		toElem->SetClass("fadein", true);
		toElem->SetProperty(Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Visible));
		toElem->SetProperty(Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::Block));

		auto backOptions = doc->GetElementById("back-options");
		if (backOptions)
		{
			backOptions->SetAttribute("data-action", to + "," + from);
		}
	}

	auto backElem = doc->GetElementById("back");
	if (backElem)
	{
		// TODO hide
	}
}

void UI_Shell::ChangeSettingsMenu(const char* setting)
{
	auto doc = ui->GetDocumentFromHandle(startDoc);
	if (!doc)
	{
		return;
	}

	idStr xAxisFunction = "settings";

	Rml::ElementList selectedElems;
	doc->QuerySelectorAll(selectedElems, ".options-select .selection");
	for (auto elem : selectedElems)
	{
		elem->SetClass("selected", false);
	}

	Rml::ElementList settingsElems;
	doc->QuerySelectorAll(settingsElems, (idStr("[data-setting='") + setting + "']").c_str());
	for (auto elem : settingsElems)
	{
		elem->SetClass("selected", true);
	}

	Rml::Element* settingsElem = doc->GetElementById(("settings-" + currentSetting).c_str());
	if( settingsElem )
	{
		settingsElem->SetProperty( Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Hidden) );
		settingsElem->SetProperty( Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::None) );
		settingsElem->SetProperty( Rml::PropertyId::Left, Rml::Property(310, Rml::Property::PX) );
		settingsElem->SetProperty( Rml::PropertyId::Opacity, Rml::Property(0, Rml::Property::NUMBER) );

	}

	Rml::Element* newSettingsElem = doc->GetElementById((idStr("settings-") + setting).c_str());
	if( newSettingsElem )
	{
		newSettingsElem->SetProperty(Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Visible));
		newSettingsElem->SetProperty(Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::Block));
		newSettingsElem->Animate("left", Rml::Property(490, Rml::Property::PX), animationTime / 8.0f / 1000.0f);
		newSettingsElem->Animate("opacity", Rml::Property(1, Rml::Property::NUMBER), animationTime / 8.0f / 1000.0f);
	}

	Rml::Element* backElem = doc->GetElementById("back");
	if( backElem )
	{
		auto dataAction = backElem->GetAttribute("data-action");
		if( dataAction && dataAction->Get<Rml::String>() != "settings-settings" )
		{
			lastBack = dataAction->Get<Rml::String>().c_str();
		}
		backElem->SetAttribute("data-action", "setting-settings");

		backElem->SetProperty(Rml::PropertyId::Visibility, Rml::Property(Rml::Style::Visibility::Hidden));
		backElem->SetProperty(Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::Block));
	}

	lastMenu = currentMenu;
	currentSetting = setting;
	currentMenu = "settings-";
	currentMenu.Append( setting );
}
#include "precompiled.h"
#pragma hdrstop

#include "RmlUserInterfaceLocal.h"

#include "renderer/GLState.h"
#include "renderer/GuiModel.h"
#include "renderer/Image.h"


RmlUserInterfaceManagerLocal rmlManagerLocal;
RmlUserInterfaceManager* rmlManager = &rmlManagerLocal;

idDeviceContext* rmlDc;

extern idCVar sys_lang;

/*
===============
RmlUserInterfaceLocal

UI to represent a context. Handles events and contains rml file loading.
===============
*/

RmlUserInterfaceLocal::RmlUserInterfaceLocal()
	: _context( nullptr )
	, _source()
	, _timeStamp( 0 )
	, _cursorX( 0 )
	, _cursorY( 0 )
	, _cursorEnabled( true )
	, _refs( 1 )
{
	_context = Rml::CreateContext("main", Rml::Vector2i(renderSystem->GetWidth(), renderSystem->GetHeight()));
	_context->EnableMouseCursor(true);
}

RmlUserInterfaceLocal::~RmlUserInterfaceLocal()
{
}

bool LoadRmlDemo( RmlUserInterfaceLocal* ui, const char* qpath )
{
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

	return true;
}

bool RmlUserInterfaceLocal::InitFromFile( const char* qpath, bool rebuild, bool cache )
{
	if( !( qpath && *qpath ) )
	{
		return false;
	}

	// First try loading the localized version
	// Then fall back to the english version
	for( int i = 0; i < 2; i++ )
	{
		_source = qpath;
		idStr trySource = qpath;
		trySource.ToLower();
		trySource.BackSlashesToSlashes();

		if( i == 0 )
		{
			trySource.Replace( "guis/", va( "guis/%s/", sys_lang.GetString() ) );
		}

		fileSystem->ReadFile( trySource, nullptr, &_timeStamp );

		if( _timeStamp != FILE_NOT_FOUND_TIMESTAMP )
		{
			_source = trySource;
			break;
		}
	}

	return true;
}

const char* RmlUserInterfaceLocal::HandleEvent( const sysEvent_t* event, int time, bool* updateVisuals )
{
	int keyModState = idRmlSystem::GetKeyModifier();

	if( event->IsMouseEvent() )
	{
		_cursorX += event->evValue;
		_cursorY += event->evValue2;

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

		_context->ProcessMouseMove( _cursorX, _cursorY, keyModState );
	}

	if( event->evValue >= K_MOUSE1 && event->evValue <= K_MOUSE16 )
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

	if( event->evValue == K_MWHEELDOWN || event->evValue == K_MWHEELUP )
	{
		float mouseWheel = ( event->evValue == K_MWHEELDOWN ) ? 1.0f : -1.0f;
		_context->ProcessMouseWheel( mouseWheel, keyModState );
	}

	if( event->IsCharEvent() )
	{
		Rml::Input::KeyIdentifier key = idRmlSystem::TranslateKey( event->evValue );

		_context->ProcessKeyDown( key, keyModState );
	}

	return "";
}

void RmlUserInterfaceLocal::HandleNamedEvent( const char* eventName )
{
}

void RmlUserInterfaceLocal::Redraw( int time, bool hud )
{
	_context->SetDimensions( Rml::Vector2i( renderSystem->GetWidth(), renderSystem->GetHeight() ) );
	_context->Update();
	( ( idRmlRender* )Rml::GetRenderInterface() )->PreRender();
	_context->Render();
	( ( idRmlRender* )Rml::GetRenderInterface() )->PostRender();
	DrawCursor();
}

RmlUi* RmlUserInterfaceLocal::AddUi(RmlUi* ui)
{
	_rmlUi.Append(ui);
	return ui;
}

void RmlUserInterfaceLocal::Reload()
{
	for (int i = 0; i < _rmlUi.Num(); i++)
	{
		_rmlUi[i]->Reload();
	}
}

const char* RmlUserInterfaceLocal::Activate( bool activate, int time )
{
	return nullptr;
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

	const idVec2 scaleToVirtual( ( float )renderSystem->GetVirtualWidth() / renderSystem->GetWidth(),
								 ( float )renderSystem->GetVirtualHeight() / renderSystem->GetHeight() );
	idVec2 bounds;
	bounds.x = _context->GetDimensions().x * scaleToVirtual.x;
	bounds.y = _context->GetDimensions().y * scaleToVirtual.y;
	float x = _cursorX * scaleToVirtual.x;
	float y = _cursorY * scaleToVirtual.y;
	rmlDc->DrawCursor( &x, &y, 36.0f * scaleToVirtual.x, bounds );
}

static void Cmd_ReloadRml(const idCmdArgs& args)
{
	rmlManagerLocal.Reload(true);
}


class MyEventListener : public Rml::EventListener
{
public:
	MyEventListener(const Rml::String& value, Rml::Element* element) : value(value), element(element) {}

	void ProcessEvent(Rml::Event& event) override
	{
		using namespace Rml;

		if (value == "exit")
		{
			// Just exit right away.
			common->Quit();
		}
	}

	void OnDetach(Rml::Element* /*element*/) override { delete this; }

private:

	Rml::String value;
	Rml::Element* element;
};

class MyEventListenerInstancer : public Rml::EventListenerInstancer
{
public:
	Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override
	{
		return new MyEventListener(value, element);
	}
};


static MyEventListenerInstancer eventInstancer;

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

	_dc.Init();

	rmlDc = &_dc;

	cmdSystem->AddCommand("reloadRml", Cmd_ReloadRml, CMD_FL_SYSTEM, "reload rml guis");

	// Register event listeners
	Rml::Factory::RegisterEventListenerInstancer(&eventInstancer);
}

void RmlUserInterfaceManagerLocal::Shutdown()
{
	int c = _guis.Num();
	for( int i = 0; i < c; i++ )
	{
		DeAlloc( _guis[i] );
	}

	_guis.Clear();

	Rml::RemoveContext("main");

	Rml::Shutdown();
}

void RmlUserInterfaceManagerLocal::Touch( const char* name )
{
}

void RmlUserInterfaceManagerLocal::BeginLevelLoad()
{
}

void RmlUserInterfaceManagerLocal::EndLevelLoad( const char* mapName )
{
	// TODO: Unload the UIs
	_dc.Init();
}

void RmlUserInterfaceManagerLocal::Preload( const char* mapName )
{
}

void RmlUserInterfaceManagerLocal::Reload( bool all )
{
	for (int i = 0; i < _guis.Num(); i++)
	{
		_guis[i]->Reload();
	}
}

void RmlUserInterfaceManagerLocal::ListGuis() const
{
}

bool RmlUserInterfaceManagerLocal::CheckGui( const char* qpath ) const
{
	return false;
}

RmlUserInterface* RmlUserInterfaceManagerLocal::Alloc() const
{
	return new( TAG_OLD_UI ) RmlUserInterfaceLocal();
}

void RmlUserInterfaceManagerLocal::DeAlloc( RmlUserInterface* gui )
{
	if( gui )
	{
		int c = _guis.Num();
		for( int i = 0; i < c; i++ )
		{
			if( _guis[i] == gui )
			{
				delete _guis[i];
				_guis.RemoveIndex( i );
				return;
			}
		}
	}
}

RmlUserInterface* RmlUserInterfaceManagerLocal::FindGui( const char* qpath, bool autoLoad, bool needUnique, bool forceNOTUnique )
{
	int c = _guis.Num();
	for( int i = 0; i < c; i++ )
	{
		RmlUserInterfaceLocal* gui = _guis[i];

		if( !gui )
		{
			continue;
		}

		if( !idStr::Icmp( gui->GetSourceFile(), qpath ) )
		{
			if( !forceNOTUnique && ( needUnique ) )
			{
				break;
			}

			// Reload the gui if it's been cleared
			if( _guis[i]->GetRefs() == 0 )
			{
				_guis[i]->InitFromFile( _guis[i]->GetSourceFile() );
			}

			_guis[i]->AddRef();
			return _guis[i];
		}
	}

	if( autoLoad )
	{
		RmlUserInterface* gui = Alloc();
		_guis.Append( (RmlUserInterfaceLocal*)gui );
		if( gui->InitFromFile( qpath ) )
		{
			return gui;
		}
		else
		{
			delete gui;
		}
	}

	return nullptr;
}

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
	return static_cast<double>( Sys_Milliseconds() ) / 1000.0;
}


bool idRmlSystem::LogMessage( Rml::Log::Type type, const Rml::String& message )
{
	if( type == Rml::Log::LT_ERROR )
	{
		common->Warning( "[RML] %s\n", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_INFO )
	{
		common->Printf( "[RML|INFO] %s\n", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_ASSERT )
	{
		common->Printf( "[RML|INFO] %s\n", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_ALWAYS )
	{
		common->Printf( "[RML|INFO] %s\n", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_DEBUG )
	{
		common->DPrintf( "[RML|INFO] %s\n", message.c_str() );
		return true;
	}

	if( type == Rml::Log::LT_WARNING )
	{
		common->Warning( "[RML|INFO] %s\n", message.c_str() );
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
		rmlDc->SetCursor( RmlUserInterfaceManagerLocal::CURSOR_ARROW );
	}
	else if( cursorName == "move" )
	{
		rmlDc->SetCursor( RmlUserInterfaceManagerLocal::CURSOR_HAND );
	}
	else if( cursorName == "pointer" )
	{
		rmlDc->SetCursor( RmlUserInterfaceManagerLocal::CURSOR_ARROW );
	}
	else if( cursorName == "resize" )
	{
		rmlDc->SetCursor( RmlUserInterfaceManagerLocal::CURSOR_HAND_JOY2 );
	}
	else if( cursorName == "cross" )
	{
		rmlDc->SetCursor( RmlUserInterfaceManagerLocal::CURSOR_ARROW );
	}
	else if( cursorName == "text" )
	{
		rmlDc->SetCursor( RmlUserInterfaceManagerLocal::CURSOR_HAND_JOY3 );
	}
	else if( cursorName == "unavailable" )
	{
		rmlDc->SetCursor( RmlUserInterfaceManagerLocal::CURSOR_ARROW );
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
			break;
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

/*
===============
idRmlRender

Front end rendering for RML
===============
*/

constexpr int kMaxInitialQuads = 1024;
constexpr int kMaxInitialVerts = kMaxInitialQuads * 4;
constexpr int kMaxInitialTris = kMaxInitialQuads * 6;

extern idGuiModel* tr_guiModel;

idRmlRender::idRmlRender()
	: _enableScissor( false )
	, _clipRects()
	, _cursorImages()
	, _numMasks( 0 )
	, _verts( nullptr )
	, _tris( nullptr )
	, _texGen( 0 )
	, _numVerts( 0 )
	, _numIndexes( 0 )
{
}

idRmlRender::~idRmlRender()
{
	delete[] _verts;
	delete[] _tris;
}

void idRmlRender::Init()
{
	_verts = new idDrawVert[kMaxInitialVerts];
	_tris = new triIndex_t[kMaxInitialTris];

	_guiSolid = declManager->FindMaterial( "guiSolid" );
}

static triIndex_t quadPicIndexes[6] = { 3, 0, 2, 2, 0, 1 };
void idRmlRender::RenderGeometry( Rml::Vertex* vertices, int numVerts, int* indices, int numIndexes, Rml::TextureHandle texture, const Rml::Vector2f& translation )
{
	triIndex_t* tris = &_tris[_numIndexes];

	for( int i = 0; i < numIndexes; i++ )
	{
		tris[i] = indices[i];
		_numIndexes++;

		if( _numIndexes > kMaxInitialTris )
		{
			// Possibly just make this dynamic
			common->FatalError( "Increase kMaxInitialTris" );
			return;
		}
	}

	const idVec2 scaleToVirtual( ( float )renderSystem->GetVirtualWidth() / renderSystem->GetWidth(),
								 ( float )renderSystem->GetVirtualHeight() / renderSystem->GetHeight() );

	idDrawVert* temp = &_verts[_numVerts];
	for( int i = 0; i < numVerts; i++ )
	{
		idVec3 pos = idVec3( vertices[i].position.x * scaleToVirtual.x, vertices[i].position.y * scaleToVirtual.y, 0 );
		pos += idVec3( translation.x * scaleToVirtual.x, translation.y * scaleToVirtual.y, 0 );
		temp[i].xyz = pos;
		temp[i].SetTexCoord( vertices[i].tex_coord.x, vertices[i].tex_coord.y );
		temp[i].SetColor( PackColor( idVec4( vertices[i].colour.red, vertices[i].colour.blue, vertices[i].colour.green, vertices[i].colour.alpha ) ) );
		temp[i].SetColor2( PackColor( idVec4( 255.0f, 255.0f, 255.0f, 255.0f ) ) );

		_numVerts++;

		if( _numVerts > kMaxInitialVerts )
		{
			common->FatalError( "Increase kMaxInitialVerts" );
			return;
		}
	}

	uint64_t glState = 0;

	if( _enableScissor )
	{
		glState = GLS_DEPTHFUNC_LESS | GLS_DEPTHMASK | GLS_STENCIL_FUNC_GEQUAL | GLS_STENCIL_MAKE_REF( 128 - _numMasks ) | GLS_STENCIL_MAKE_MASK( 255 );
	}

	idDrawVert* verts = tr_guiModel->AllocTris(
							numVerts,
							tris,
							numIndexes,
							reinterpret_cast<const idMaterial*>( texture ),
							glState,
							STEREO_DEPTH_TYPE_NONE );

	WriteDrawVerts16( verts, temp, numVerts );

	_numVerts = 0;
	_numIndexes = 0;
}

void idRmlRender::RenderClipMask()
{
	// Usually, scissor regions are handled  with actual scissor render commands.
	// We're using stencil masks to do the same thing because it works in worldspace a
	// lot better than screen space scissor rects.
	const idVec2 scaleToVirtual( ( float )renderSystem->GetVirtualWidth() / renderSystem->GetWidth(),
								 ( float )renderSystem->GetVirtualHeight() / renderSystem->GetHeight() );

	ALIGNTYPE16 idDrawVert localVerts[4];

	localVerts[0].Clear();
	localVerts[0].xyz[0] = _clipRects.x * scaleToVirtual.x;
	localVerts[0].xyz[1] = _clipRects.y * scaleToVirtual.y;
	localVerts[0].xyz[2] = 0.0f;
	localVerts[0].SetTexCoord( 0.0f, 1.0f );
	localVerts[0].SetColor( PackColor( idVec4() ) );
	localVerts[0].ClearColor2();

	localVerts[1].Clear();
	localVerts[1].xyz[0] = ( _clipRects.x + _clipRects.w ) * scaleToVirtual.x;
	localVerts[1].xyz[1] = _clipRects.y * scaleToVirtual.y;
	localVerts[1].xyz[2] = 0.0f;
	localVerts[1].SetTexCoord( 1.0f, 1.0f );
	localVerts[1].SetColor( PackColor( idVec4() ) );
	localVerts[1].ClearColor2();

	localVerts[2].Clear();
	localVerts[2].xyz[0] = ( _clipRects.x + _clipRects.w ) * scaleToVirtual.x;
	localVerts[2].xyz[1] = ( _clipRects.y + _clipRects.h ) * scaleToVirtual.y;
	localVerts[2].xyz[2] = 0.0f;
	localVerts[2].SetTexCoord( 1.0f, 0.0f );
	localVerts[2].SetColor( PackColor( idVec4() ) );
	localVerts[2].ClearColor2();

	localVerts[3].Clear();
	localVerts[3].xyz[0] = _clipRects.x * scaleToVirtual.x;
	localVerts[3].xyz[1] = ( _clipRects.y + _clipRects.h ) * scaleToVirtual.y;
	localVerts[3].xyz[2] = 0.0f;
	localVerts[3].SetTexCoord( 0.0f, 0.0f );
	localVerts[3].SetColor( PackColor( idVec4() ) );
	localVerts[3].ClearColor2();

	uint64_t glState = 0;

	if( _enableScissor && _numMasks == 0 )
	{
		// Nothing written to the stencil buffer yet. Initially seed it with the first clipping rectangle
		glState = GLS_COLORMASK | GLS_ALPHAMASK | GLS_STENCIL_FUNC_ALWAYS | GLS_STENCIL_MAKE_REF( 128 ) | GLS_STENCIL_MAKE_MASK( 255 );
	}
	else
	{
		glState = GLS_COLORMASK | GLS_ALPHAMASK | GLS_STENCIL_OP_FAIL_KEEP | GLS_STENCIL_OP_ZFAIL_KEEP | GLS_STENCIL_OP_PASS_DECR;
	}

	idDrawVert* maskVerts = tr_guiModel->AllocTris(
								4,
								quadPicIndexes,
								6,
								reinterpret_cast<const idMaterial*>( _guiSolid ),
								glState,
								STEREO_DEPTH_TYPE_NONE );

	WriteDrawVerts16( maskVerts, localVerts, 4 );
}

void idRmlRender::SetScissorRegion( int x, int y, int width, int height )
{
	_clipRects = idRectangle( x, y, width, height );
}

bool idRmlRender::LoadTexture( Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source )
{
	const idMaterial* material = declManager->FindMaterial( source.c_str(), true );
	material->SetSort( SS_GUI );

	if( !material )
	{
		return false;
	}

	material->ReloadImages( false );

	texture_handle = reinterpret_cast<Rml::TextureHandle>( material );
	texture_dimensions.x = material->GetImageWidth();
	texture_dimensions.y = material->GetImageHeight();

	return true;
}

bool idRmlRender::GenerateTexture( Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions )
{
	idImage* image = globalImages->AllocImage( va( "_rmlImage%d", _texGen ) );

	image->GenerateImage( source, source_dimensions.x, source_dimensions.y, textureFilter_t::TF_NEAREST,
						  textureRepeat_t::TR_CLAMP,
						  textureUsage_t::TD_LOOKUP_TABLE_RGBA );

	const idMaterial* material = declManager->FindMaterial( va( "_rmlImage%d", _texGen ) );
	material->SetSort( SS_GUI );

	texture_handle = reinterpret_cast<Rml::TextureHandle>( material );

	_texGen++;

	return material != nullptr;
}

void idRmlRender::EnableScissorRegion( bool enable )
{
	_enableScissor = enable;

	if( _clipRects.w <= 0 || _clipRects.h <= 0 )
	{
		return;
	}

	if( !_enableScissor )
	{
		return;
	}

	_numMasks++;

	RenderClipMask();
}

void idRmlRender::PreRender()
{
	_numMasks = 0;
}

void idRmlRender::PostRender()
{
}

/*
===============
RmlFileSystem

File system for RML
===============
*/


RmlFileSystem::RmlFileSystem()
{
}

RmlFileSystem::~RmlFileSystem()
{
}

Rml::FileHandle RmlFileSystem::Open( const Rml::String& path )
{
	idFile* file = fileSystem->OpenFileRead( path.c_str() );

	return Rml::FileHandle( file );
}

void RmlFileSystem::Close( Rml::FileHandle file )
{
	if( file )
	{
		fileSystem->CloseFile( reinterpret_cast<idFile*>( file ) );
	}
}

size_t RmlFileSystem::Read( void* buffer, size_t size, Rml::FileHandle file )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return 0;
	}

	return theFile->Read( buffer, size );
}

bool RmlFileSystem::Seek( Rml::FileHandle file, long offset, int origin )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return false;
	}

	return theFile->Seek( offset, ( fsOrigin_t )origin );
}

size_t RmlFileSystem::Tell( Rml::FileHandle file )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return 0;
	}

	return theFile->Tell();
}

size_t RmlFileSystem::Length( Rml::FileHandle file )
{
	idFile* theFile = reinterpret_cast<idFile*>( file );
	if( !theFile )
	{
		return 0;
	}

	return theFile->Length();
}

bool RmlFileSystem::LoadFile( const Rml::String& path, Rml::String& out_data )
{
	idFile* theFile = fileSystem->OpenFileRead( path.c_str() );

	if( !theFile )
	{
		return false;
	}

	auto len = theFile->Length();
	out_data.resize( len );
	auto ret = theFile->Read( &out_data[0], len );
	return ret;
}
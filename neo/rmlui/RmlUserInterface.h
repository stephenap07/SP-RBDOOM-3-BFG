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

// This class provides an interface to manage the rml documents. It provides access to reload documents and handle material loading as you edit them in real time.
class RmlUserInterface
{
public:
	virtual ~RmlUserInterface() = default;

	virtual bool					Init( const char* name, idSoundWorld* soundWorld ) = 0;

	// handles an event, can return an action string, the caller interprets
	// any return and acts accordingly
	virtual const char*				HandleEvent( const sysEvent_t* event, int time, bool* updateVisuals = NULL ) = 0;

	// handles a named event
	virtual void					HandleNamedEvent( const char* eventName ) = 0;

	// repaints the ui
	virtual void					Redraw( int time, bool hud = false ) = 0;

	virtual Rml::ElementDocument*	LoadDocument( const char* filePath ) = 0;

	virtual bool					IsDocumentOpen( const char* name) = 0;

	virtual void					CloseDocument( const char* name) = 0;

	// cursor
	virtual bool					IsCursorEnabled( ) const = 0;
	virtual void					SetCursorEnabled( bool _enabled = true ) = 0;

	virtual void					SetCursor( float x, float y ) = 0;

	virtual float					CursorX() = 0;

	virtual float					CursorY() = 0;

	// Activated the gui.
	virtual const char*				Activate( bool activate, int time ) = 0;

	virtual Rml::Context*			Context() = 0;

	virtual bool					IsActive() = 0;

	virtual bool					IsPausingGame() = 0;

	virtual void					SetIsPausingGame( bool pause ) = 0;

	virtual bool					InhibitsControl() = 0;

	virtual void					SetInhibitsControl( bool inhibit ) = 0;

	virtual Rml::ElementDocument*	SetNextScreen( const char* _nextScreen ) = 0;

	virtual void					HideAllDocuments() = 0;

	// Window specific

	virtual idVec2					GetScreenSize() const = 0;
	virtual void					SetSize(int width, int height) = 0;
	virtual void					SetUseScreenResolution(bool useScreen) = 0;

	// Sound
	virtual int						PlaySound( const char* sound, int channel = SCHANNEL_ANY, bool blocking = false ) = 0;

	virtual void					StopSound( int channel = SCHANNEL_ANY ) = 0;
};

class RmlUserInterfaceManager
{
public:
	virtual							~RmlUserInterfaceManager() = default;

	virtual void					Init() = 0;
	virtual void					Shutdown() = 0;
	virtual RmlUserInterface*		Find( const char* name, bool autoload ) = 0;
	virtual RmlUserInterface*		Find( const Rml::Context* context ) = 0;

	virtual void					BeginLevelLoad() = 0;
	virtual void					EndLevelLoad( const char* mapName ) = 0;
	virtual bool					InLevelLoad() const = 0;

	// Reloads changed guis, or all guis.
	virtual void					Reload( bool all ) = 0;

	virtual void					PostRender() = 0;
};

extern RmlUserInterfaceManager* rmlManager;

#endif
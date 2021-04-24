#ifndef __RMLUSERINTERFACE_H__
#define __RMLUSERINTERFACE_H__

class RmlUi
{
public:
	virtual bool Reload() = 0;
	virtual bool InhibitsControl() = 0;
};

class RmlUserInterface
{
public:
	virtual ~RmlUserInterface() = default;

	virtual bool				Init( const char* name ) = 0;

	// handles an event, can return an action string, the caller interprets
	// any return and acts accordingly
	virtual const char*			HandleEvent( const sysEvent_t* event, int time, bool* updateVisuals = NULL ) = 0;

	// handles a named event
	virtual void				HandleNamedEvent( const char* eventName ) = 0;

	// repaints the ui
	virtual void				Redraw( int time, bool hud = false ) = 0;

	virtual RmlUi*				AddUi( RmlUi* ui ) = 0;

	virtual void				SetCursor( float x, float y ) = 0;

	virtual float				CursorX() = 0;

	virtual float				CursorY() = 0;

	// Activated the gui.
	virtual const char*			Activate( bool activate, int time ) = 0;

	virtual Rml::Context*		Context() = 0;

	virtual bool				IsActive() = 0;

	virtual bool				IsPausingGame() = 0;

	virtual void				SetIsPausingGame( bool pause ) = 0;

	virtual bool				InhibitsControl() = 0;

	virtual void				SetInhibitsControl( bool inhibit ) = 0;
};

class RmlUserInterfaceManager
{
public:
	virtual						~RmlUserInterfaceManager() = default;

	virtual void				Init() = 0;
	virtual void				Shutdown() = 0;
	virtual RmlUserInterface*	Find( const char* name, bool autoload ) = 0;

	virtual void				BeginLevelLoad() = 0;
	virtual void				EndLevelLoad( const char* mapName ) = 0;
	virtual bool				InLevelLoad() const = 0;

	// Reloads changed guis, or all guis.
	virtual void				Reload( bool all ) = 0;

	virtual void				PostRender() = 0;
};

extern RmlUserInterfaceManager* rmlManager;

#endif
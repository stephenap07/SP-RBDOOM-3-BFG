#ifndef __RMLUSERINTERFACE_H__
#define __RMLUSERINTERFACE_H__

class RmlUi
{
public:
	virtual bool Reload() = 0;
};

class RmlUserInterface
{
public:
	virtual ~RmlUserInterface() = default;

	virtual bool				InitFromFile( const char* qpath, bool rebuild = true, bool cache = true ) = 0;

	// handles an event, can return an action string, the caller interprets
	// any return and acts accordingly
	virtual const char*			HandleEvent( const sysEvent_t* event, int time, bool* updateVisuals = NULL ) = 0;

	// handles a named event
	virtual void				HandleNamedEvent( const char* eventName ) = 0;

	// repaints the ui
	virtual void				Redraw( int time, bool hud = false ) = 0;

	virtual RmlUi*				AddUi(RmlUi* ui) = 0;

	virtual void				SetCursor( float x, float y ) = 0;

	virtual float				CursorX() = 0;

	virtual float				CursorY() = 0;

	// Activated the gui.
	virtual const char*			Activate( bool activate, int time ) = 0;

	virtual Rml::Context*		Context() = 0;
};

class RmlUserInterfaceManager
{
public:
	virtual						~RmlUserInterfaceManager() = default;

	virtual void				Init() = 0;
	virtual void				Shutdown() = 0;
	virtual void				Touch( const char* name ) = 0;

	virtual void				BeginLevelLoad() = 0;
	virtual void				EndLevelLoad( const char* mapName ) = 0;
	virtual void				Preload( const char* mapName ) = 0;

	// Reloads changed guis, or all guis.
	virtual void				Reload( bool all ) = 0;

	// lists all guis
	virtual void				ListGuis() const = 0;

	// Returns true if gui exists.
	virtual bool				CheckGui( const char* qpath ) const = 0;

	// Allocates a new gui.
	virtual RmlUserInterface*	Alloc() const = 0;

	// De-allocates a gui.. ONLY USE FOR PRECACHING
	virtual void				DeAlloc( RmlUserInterface* gui ) = 0;

	// Returns NULL if gui by that name does not exist.
	virtual RmlUserInterface*	FindGui( const char* qpath, bool autoLoad = false, bool needUnique = false, bool forceUnique = false ) = 0;
};

extern RmlUserInterfaceManager* rmlManager;

#endif
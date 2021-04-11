#ifndef __RMLUSERINTERFACELOCAL_H__
#define __RMLUSERINTERFACELOCAL_H__

#include "RmlUserInterface.h"

#include "RmlUi/Core.h"
#include "RmlFileSystem.h"
#include "D3RmlRender.h"

#include "ui/D3RmlSystem.h"
#include "ui/DeviceContext.h"


class RmlUserInterfaceLocal : public RmlUserInterface
{
public:

	RmlUserInterfaceLocal();

	virtual ~RmlUserInterfaceLocal();

	// Begin RmlUserInterface
	bool						InitFromFile( const char* qpath, bool rebuild = true, bool cache = true ) override;

	// handles an event, can return an action string, the caller interprets
	// any return and acts accordingly
	const char*					HandleEvent( const sysEvent_t* event, int time, bool* updateVisuals = NULL ) override;

	// handles a named event
	void						HandleNamedEvent( const char* eventName ) override;

	// repaints the ui
	void						Redraw( int time, bool hud = false ) override;

	RmlUi*						AddUi(RmlUi* ui);

	void						Reload();

	void						SetCursor( float x, float y ) override
	{
		_cursorX = x;
		_cursorY = y;
	}

	float						CursorX() override
	{
		return _cursorX;
	}

	float						CursorY() override
	{
		return _cursorY;
	}

	bool						IsCursorEnabled()
	{
		return _cursorEnabled;
	}

	bool						SetCursorEnabled( bool cursorEnabled )
	{
		_cursorEnabled = cursorEnabled;
	}

	// Activated the gui.
	const char*					Activate( bool activate, int time ) override;

	// End RmlUserInterface

	size_t						Size();

	const char*					GetSourceFile() const
	{
		return _source;
	}

	ID_TIME_T					GetTimeStamp() const
	{
		return _timeStamp;
	}

	void						ClearRefs()
	{
		_refs = 0;
	}

	void						AddRef()
	{
		_refs++;
	}

	int							GetRefs()
	{
		return _refs;
	}

	Rml::Context*				Context()
	{
		return _context;
	}

	void						DrawCursor();

private:

	friend bool LoadRmlDemo( RmlUserInterfaceLocal* ui, const char* qpath );

	Rml::Context*				_context;

	idStr						_source;
	ID_TIME_T					_timeStamp;

	float						_cursorX;
	float						_cursorY;
	bool						_cursorEnabled;

	int							_refs;

	idList<RmlUi*>				_rmlUi;
};

class RmlUserInterfaceManagerLocal : public RmlUserInterfaceManager
{
public:

	enum
	{
		CURSOR_ARROW,
		CURSOR_HAND,
		CURSOR_HAND_JOY1,
		CURSOR_HAND_JOY2,
		CURSOR_HAND_JOY3,
		CURSOR_HAND_JOY4,
		CURSOR_COUNT
	};

	virtual					~RmlUserInterfaceManagerLocal() = default;

	void					Init() override;
	void					Shutdown() override;
	void					Touch( const char* name ) override;

	void					BeginLevelLoad() override;
	void					EndLevelLoad( const char* mapName ) override;
	void					Preload( const char* mapName ) override;

	// Reloads changed guis, or all guis.
	void					Reload( bool all ) override;

	// lists all guis
	void					ListGuis() const override;

	// Returns true if gui exists.
	bool					CheckGui( const char* qpath ) const override;

	// Allocates a new gui.
	RmlUserInterface*		Alloc() const override;

	// De-allocates a gui.. ONLY USE FOR PRECACHING
	void					DeAlloc( RmlUserInterface* gui ) override;

	// Returns NULL if gui by that name does not exist.
	RmlUserInterface*		FindGui( const char* qpath, bool autoLoad = false, bool needUnique = false, bool forceUnique = false ) override;

private:

	idDeviceContextOptimized		_dc;

	idList<RmlUserInterfaceLocal*>	_guis;

	idRmlSystem						_rmlSystem;
	idRmlRender						_rmlRender;
	RmlFileSystem					_rmlFileSystem;
};

#endif  // !__RMLUSERINTERFACE_H__
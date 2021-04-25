#ifndef __RMLUSERINTERFACELOCAL_H__
#define __RMLUSERINTERFACELOCAL_H__

#include "RmlUserInterface.h"

#include "RmlUi/Core.h"
#include "RmlFileSystem.h"
#include "D3RmlRender.h"

#include "rmlui/RmlSystem.h"
#include "ui/DeviceContext.h"

class RmlUserInterfaceLocal : public RmlUserInterface
{
public:

	RmlUserInterfaceLocal();

	virtual ~RmlUserInterfaceLocal();

	// Begin RmlUserInterface
	bool						Init( const char* name ) override;

	// handles an event, can return an action string, the caller interprets
	// any return and acts accordingly
	const char*					HandleEvent( const sysEvent_t* event, int time, bool* updateVisuals = NULL ) override;

	// handles a named event
	void						HandleNamedEvent( const char* eventName ) override;

	// repaints the ui
	void						Redraw( int time, bool hud = false ) override;

	RmlUi*						AddUi( RmlUi* ui );

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

	void						SetCursorEnabled( bool cursorEnabled )
	{
		_cursorEnabled = cursorEnabled;
	}

	// Activated the gui.
	const char*					Activate( bool activate, int time ) override;

	// End RmlUserInterface

	size_t						Size();

	const char*					GetName() const
	{
		return _name;
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

	bool						IsActive() override
	{
		return _isActive;
	}

	bool						IsPausingGame() override
	{
		return _isPausingGame;
	}

	virtual void				SetIsPausingGame( bool pause ) override
	{
		_isPausingGame = pause;
	}

	bool						InhibitsControl() override
	{
		return _inhibitsControl;
	}

	void						SetInhibitsControl( bool inhibit )
	{
		_inhibitsControl = inhibit;
	}

protected:

	Rml::Context*				_context;

	idStr						_name;
	ID_TIME_T					_timeStamp;

	float						_cursorX;
	float						_cursorY;
	bool						_cursorEnabled;
	bool						_isActive;
	bool						_isPausingGame;
	bool						_inhibitsControl;

	int							_refs;

	idList<RmlUi*>				_rmlUi;
};

struct RmlMaterial
{
	idImage* image = nullptr;
	const idMaterial* material = nullptr;
	const byte* data = nullptr;
	idVec2 dimensions = idVec2( 0.0f, 0.0f );

	~RmlMaterial();
};

class RmlUserInterfaceManagerLocal : public RmlUserInterfaceManager
{
public:

	virtual						~RmlUserInterfaceManagerLocal() = default;

	void						Init() override;
	void						Shutdown() override;
	virtual RmlUserInterface*	Find( const char* name, bool autoload ) override;

	void						BeginLevelLoad() override;
	void						EndLevelLoad( const char* mapName ) override;
	bool						InLevelLoad() const override
	{
		return _inLevelLoad;
	}

	// Reloads changed guis, or all guis.
	void						Reload( bool all ) override;

	void						PostRender() override;

	// Class owns data
	void						AddMaterialToReload( const idMaterial* material, idImage* image, idVec2 dimensions, const byte* data );

private:

	idDeviceContextOptimized		_dc;

	idList<RmlUserInterfaceLocal*>	_guis;
	idList<RmlMaterial>				_materialsToReload;

	idRmlSystem						_rmlSystem;
	idRmlRender						_rmlRender;
	RmlFileSystem					_rmlFileSystem;
	bool							_inLevelLoad;
};

#endif  // !__RMLUSERINTERFACE_H__
#ifndef __GUI_TESTGUI_H__
#define __GUI_TESTGUI_H__


class RmlUserInterfaceLocal;

namespace Rml
{
class EventListenerInstancer;
class ElementDocument;
}

class UI_Shell
{
public:

	UI_Shell();

	bool					Init();

	bool					IsActive()
	{
		return _isActive;
	}

	bool					IsPausingGame()
	{
		return _isPausingGame;
	}
	void					SetIsPausingGame( bool pause )
	{
		_isPausingGame = pause;
	}

	bool					InhibitsControl()
	{
		return _inhibitsControl;
	}

	void					SetInhibitsControl( bool inhibits )
	{
		_inhibitsControl = inhibits;
	}

	bool					IsCursorEnabled() const;

	void					SetCursorEnabled( bool showCursor );

	bool					HandleEvent( const sysEvent_t* event, int time );

	void					Redraw( int time );

	void					SetNextScreen( const char* screen );

	void					TransitionNextScreen();

	Rml::ElementDocument*	LoadDocument( const char* windowName );

	RmlUserInterfaceLocal*	Ui()
	{
		return _ui;
	}

private:

	Rml::EventListenerInstancer*	_eventListenerInstancer;
	idStr							_nextScreen;
	idStr							_currentScreen;

	RmlUserInterfaceLocal*			_ui;

	bool							_isActive;
	bool							_isPausingGame;
	bool							_inhibitsControl;
	bool							_showCursor;
};

#endif
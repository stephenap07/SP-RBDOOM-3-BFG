#ifndef __GUI_TESTGUI_H__
#define __GUI_TESTGUI_H__

#include "rmlui/RmlUserInterfaceLocal.h"

class UI_Shell;
class MyEventListener : public Rml::EventListener
{
public:
	MyEventListener( UI_Shell* shell, const Rml::String& value, Rml::Element* element );

	void ProcessEvent( Rml::Event& event ) override;

	void OnDetach( Rml::Element* /*element*/ ) override;

private:
	UI_Shell* _shell;
	Rml::String value;
	Rml::Element* element;
};

class MyEventListenerInstancer : public Rml::EventListenerInstancer
{
public:
	MyEventListenerInstancer( UI_Shell* shell )
		: _shell( shell )
	{
	}

	Rml::EventListener* InstanceEventListener( const Rml::String& value, Rml::Element* element ) override
	{
		return new MyEventListener( _shell, value, element );
	}

private:
	UI_Shell* _shell;
};

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

	bool					IsCursorEnabled() const
	{
		return _ui->IsCursorEnabled();
	}

	void					SetCursorEnabled( bool showCursor )
	{
		_ui->SetCursorEnabled( showCursor );
	}

	bool					HandleEvent( const sysEvent_t* event, int time )
	{
		return _ui->HandleEvent( event, time );
	}

	void					Redraw( int time );

	void					SetNextScreen( const char* screen );

	void					TransitionNextScreen();

	Rml::ElementDocument*	LoadDocument( const char* windowName );

	RmlUserInterfaceLocal*	Ui()
	{
		return _ui;
	}

private:

	MyEventListenerInstancer	_eventListenerInstancer;
	idStr						_nextScreen;
	idStr						_currentScreen;

	RmlUserInterfaceLocal*		_ui;

	bool						_isActive;
	bool						_isPausingGame;
	bool						_inhibitsControl;
	bool						_showCursor;
};

#endif
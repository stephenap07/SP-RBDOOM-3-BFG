#ifndef __GUI_RMLSHELL_H__
#define __GUI_RMLSHELL_H__

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

	bool					Init( );

	Rml::ElementDocument*	SetNextScreen( const char* name );

	RmlUserInterface*		Ui()
	{
		return _ui;
	}

private:

	// The container of the ui.
	RmlUserInterface*		_ui;
};

#endif
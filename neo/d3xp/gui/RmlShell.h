#ifndef __GUI_RMLSHELL_H__
#define __GUI_RMLSHELL_H__

#ifndef __TYPEINFOGEN__
#include "../../renderer/RenderCommon.h"
#include "rmlui/Core.h"
#endif

#include <vector>

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

	~UI_Shell();

	bool					Init( );

	void SetupDataBinding( );

	Rml::ElementDocument*	SetNextScreen( const char* name );

	RmlUserInterface*		Ui()
	{
		return _ui;
	}

private:

	// The container of the ui.
	RmlUserInterface*			_ui;

	idStrStatic<512>			_previousScreen;
	idStrStatic<512>			_currentScreen;

	Rml::DataModelHandle		vidModeModel;
	std::vector<vidMode_t>		modeList;
};

#endif
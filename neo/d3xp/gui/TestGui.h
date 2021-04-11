#ifndef __GUI_TESTGUI_H__
#define __GUI_TESTGUI_H__

#include "rmlui/RmlUserInterfaceLocal.h"

class UI_Inventory : public RmlUi
{
public:

	UI_Inventory();

	bool Init(const char* title, idVec2 position, Rml::Context* context);

	bool Shutdown();

	bool Reload() override;

	// Adds a brand-new item into this inventory.
	void AddItem(const char* name);

private:

	void AddItem_Rml(const char* name);

	ID_TIME_T				_timeStamp;
	idStr					_title;
	idVec2					_position;
	Rml::ElementDocument*	_document;
	Rml::Context*			_context;

	idList<idStr>			_items;
};

class UI_MainMenu : public RmlUi
{
public:

	UI_MainMenu();

	bool Init(const char* title, idVec2 position, Rml::Context* context);

	bool Reload() override;

private:

	Rml::Context* _context;
	Rml::ElementDocument* _document;
};
#endif
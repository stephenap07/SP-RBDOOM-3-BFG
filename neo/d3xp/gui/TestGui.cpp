/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "rmlui/RmlUserInterfaceLocal.h"

#include "TestGui.h"

// UI User code

UI_Inventory::UI_Inventory()
	: _timeStamp(0)
	, _title()
	, _position(0.0f, 0.0f)
	, _document(nullptr)
	, _context(nullptr)
	, _items()
{
}

bool UI_Inventory::Init(const char* title, idVec2 position, Rml::Context* context)
{
	_context = context;
	_document = context->LoadDocument("guis/rml/inventory/inventory.rml");
	_title = title;

	if (_document)
	{
		//_document->GetElementById( "title" )->SetInnerRML( title );
		_document->SetProperty(Rml::PropertyId::Left, Rml::Property(50.0f, Rml::Property::PERCENT));
		_document->SetProperty(Rml::PropertyId::Top, Rml::Property(50.0f, Rml::Property::PERCENT));
		_document->Show();

		common->Printf("Loaded document %s", _document->GetId().c_str());

		return true;
	}
	else
	{
		common->Error("Failed to load test document");
		return false;
	}
}

bool UI_Inventory::Shutdown()
{
	if (_document)
	{
		delete _document;
	}

	return true;
}

bool UI_Inventory::Reload()
{
	_context->UnloadDocument(_document);
	_document = _context->LoadDocument("guis/rml/inventory/inventory.rml");

	if (_document)
	{
		//_document->GetElementById("title")->SetInnerRML(_title.c_str());
		//_document->SetProperty(Rml::PropertyId::Left, Rml::Property(_position.x, Rml::Property::DP));
		//_document->SetProperty(Rml::PropertyId::Top, Rml::Property(_position.y, Rml::Property::DP));
		_document->Show();

		for (int i = 0; i < _items.Num(); i++)
		{
			AddItem_Rml(_items[i].c_str());
		}

		common->Printf("Loaded document %s", _document->GetId().c_str());

		return true;
	}

	common->Error("Failed to load test document");
	return false;
}

void UI_Inventory::AddItem(const char* name)
{
	_items.Append(name);
	AddItem_Rml(name);
}

void UI_Inventory::AddItem_Rml(const char* name)
{
	if (!_document)
	{
		return;
	}

	Rml::Element* content = _document->GetElementById("content");
	if (!content)
	{
		return;
	}

	// Create the new 'icon' element.
	Rml::ElementPtr icon = Rml::Factory::InstanceElement(content, "icon", "icon", Rml::XMLAttributes());
	icon->SetInnerRML(name);
	content->AppendChild(std::move(icon));
}


UI_MainMenu::UI_MainMenu()
	: _document(nullptr)
{
}

bool UI_MainMenu::Init(const char* title, idVec2 position, Rml::Context* context)
{
	_context = context;
	_document = _context->LoadDocument("guis/rml/inventory/inventory.rml");

	if (_document)
	{
		common->Printf("Loaded document %s", _document->GetId().c_str());

		return true;
	}
	else
	{
		common->Error("Failed to load test document");
		return false;
	}
}

bool UI_MainMenu::Reload()
{
	_document = _context->LoadDocument("guis/rml/inventory/inventory.rml");

	if (_document)
	{
		common->Printf("Loaded document %s", _document->GetId().c_str());

		return true;
	}
	else
	{
		common->Error("Failed to load test document");
		return false;
	}
}


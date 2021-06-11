#ifndef __GUI_GLOBALRMLEVENTLISTENER_H__
#define __GUI_GLOBALRMLEVENTLISTENER_H__


#include "rmlui/RmlUserInterfaceLocal.h"

#include "RmlUi/Core/EventListener.h"
#include "RmlUi/Core/EventListenerInstancer.h"


///
/// This event listener responds to commands from the rml gui stuff.
class GlobalRmlEventListener : public Rml::EventListener
{
public:
	GlobalRmlEventListener( RmlUserInterface* _ui, const Rml::String& value, Rml::Element* element );

	void ProcessEvent( Rml::Event& event ) override;

	void OnDetach( Rml::Element* element ) override;

private:
	RmlUserInterface* ui;
	Rml::String value;
	Rml::Element* element;
};

#endif
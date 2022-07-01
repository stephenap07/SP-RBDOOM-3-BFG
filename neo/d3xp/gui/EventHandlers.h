#pragma once

#include "rmlui/RmlEventHandler.h"

class UI_Shell;

class RmlGameEventHandler : public RmlEventHandler
{
public:
	RmlGameEventHandler( UI_Shell* _shell )
		: shell( _shell )
	{
	}

	virtual ~RmlGameEventHandler( ) = default;

	void ProcessEvent( Rml::Event& _event, idLexer& src, idToken& token ) override;

protected:

	UI_Shell* shell;
};

class EventHandlerOptions : public RmlGameEventHandler
{
public:

	EventHandlerOptions( UI_Shell* _shell )
		: RmlGameEventHandler( _shell )
	{
	}

	~EventHandlerOptions( ) override = default;

	void ProcessEvent( Rml::Event& event, idLexer& src, idToken& token ) override;
};
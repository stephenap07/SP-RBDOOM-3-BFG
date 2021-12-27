#ifndef __GUI_GAMERMLEVENTHANDLER_H__
#define __GUI_GAMERMLEVENTHANDLER_H__

#include "rmlui/RmlEventHandler.h"

class GameRmlEventHandler : public RmlEventHandler
{
public:
	GameRmlEventHandler();

	virtual ~GameRmlEventHandler();

	void ProcessEvent( Rml::Event& _event, idLexer& src, idToken& token ) override;
};

#endif
#pragma once

extern const idEventDef EV_LuaEntity_PlayAnim;

class LuaEntity : public idAnimatedEntity
{
public:
	CLASS_PROTOTYPE( LuaEntity );

	LuaEntity( );

	virtual ~LuaEntity();

	void Spawn( );

	virtual void			Think();

	// Script Events
	void					Event_PlayAnim( int channel, const char* animname, bool loop );

private:

	int animBlendFrames;
	int animDoneTime;
};


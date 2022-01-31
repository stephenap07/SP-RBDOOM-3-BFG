#pragma hdrstop
#include "precompiled.h"

#include "../Game_local.h"

const idEventDef EV_LuaEntity_PlayAnim( "playAnimation", "dsd" );

CLASS_DECLARATION( idAnimatedEntity, LuaEntity )
EVENT( EV_LuaEntity_PlayAnim, LuaEntity::Event_PlayAnim )
END_CLASS

LuaEntity::LuaEntity( )
	: animBlendFrames( 2 )
	, animDoneTime( 0 )
{
}

LuaEntity::~LuaEntity( )
{
}

void LuaEntity::Spawn( )
{
}

void LuaEntity::Event_PlayAnim( int channel, const char* animname, bool loop )
{
	int anim;

	anim = animator.GetAnim( animname );
	if( !anim )
	{
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname, name.c_str( ), GetEntityDefName( ) );
		animator.Clear( channel, gameLocal.time, FRAME2MS( animBlendFrames ) );
		animDoneTime = 0;
	}
	else
	{
		if( loop )
		{
			animator.CycleAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		}
		else
		{
			animator.PlayAnim( channel, anim, gameLocal.time, FRAME2MS( animBlendFrames ) );
		}

		animDoneTime = animator.CurrentAnim( channel )->GetEndTime( );
	}

	//animBlendFrames = 0;

	gameLocal.scriptManager.ReturnInt( 0 );
}

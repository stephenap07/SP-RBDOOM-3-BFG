/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) Stephen Pridham

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

#include "../Game_local.h"

const idEventDef EV_LuaEntity_PlayAnim( "playAnimation", "dsd" );

CLASS_DECLARATION( idAnimatedEntity, LuaEntity )
EVENT( EV_LuaEntity_PlayAnim, LuaEntity::Event_PlayAnim )
END_CLASS

LuaEntity::LuaEntity( )
	: _animBlendFrames( 2 )
	, _animDoneTime( 0 )
{
	fl.networkSync = true;
}

LuaEntity::~LuaEntity( )
{
}

void LuaEntity::Spawn( )
{
	GetPhysics()->SetContents( CONTENTS_SOLID );
}

void LuaEntity::Think()
{
	idAnimatedEntity::Think();
	stateScript.Think();
}

void LuaEntity::Event_PlayAnim( int channel_, const char* animname_, bool loop_ )
{
	const int anim = animator.GetAnim( animname_ );

	if( !anim )
	{
		gameLocal.Warning( "missing '%s' animation on '%s' (%s)", animname_, name.c_str( ), GetEntityDefName( ) );
		animator.Clear( channel_, gameLocal.time, FRAME2MS( _animBlendFrames ) );
		_animDoneTime = 0;
	}
	else
	{
		if( loop_ )
		{
			animator.CycleAnim( channel_, anim, gameLocal.time, FRAME2MS( _animBlendFrames ) );
		}
		else
		{
			animator.PlayAnim( channel_, anim, gameLocal.time, FRAME2MS( _animBlendFrames ) );
		}

		_animDoneTime = animator.CurrentAnim( channel_ )->GetEndTime();
	}

	//animBlendFrames = 0;

	gameLocal.scriptManager.ReturnInt( 0 );
}

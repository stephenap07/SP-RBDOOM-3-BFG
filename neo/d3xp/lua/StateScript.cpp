#include "precompiled.h"
#pragma hdrstop

/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Stephen Pridham

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

#include "StateScript.h"

#include "d3xp/Game_local.h"

#include <lua.hpp>

// Lua Interface
static const char* ModuleName = "EntitySystem";
static const char* RegisterName = "Register";
static const char* SendEventName = "SendEvent";
static const char* ThinkName = "Think";


/**
* Entity registration and lifetime management.
*/
static void TS_New( lua_State* L, idEntity* ent )
{
	// Get the register table
	lua_getglobal( L, ModuleName );
	if( !lua_istable( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find the %s table", ModuleName );
		return;
	}

	lua_getfield( L, -1, RegisterName );
	if( !lua_isfunction( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find function %s", RegisterName );
	}

	gameLocal.scriptManager.ReturnEntity( ent );

	if( lua_pcall( L, 1, 0, 0 ) != LUA_OK )
	{
		gameLocal.Warning( "Something went wrong in the %s.%s: %s", ModuleName, RegisterName, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}
}

static void TS_SendEvent( lua_State* L, idEntity* ent, const char* eventName )
{
	// Get the register table
	lua_getglobal( L, ModuleName );
	if( !lua_istable( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find the %s table", ModuleName );
		return;
	}

	lua_getfield( L, -1, SendEventName );
	gameLocal.scriptManager.ReturnEntity( ent );

	lua_createtable( L, 0, 1 );
	lua_pushstring( L, eventName );
	lua_setfield( L, -2, "name" );

	if( lua_pcall( L, 2, 0, 0 ) != LUA_OK )
	{
		gameLocal.Warning( "Something went wrong in the %s.%s: %s", ModuleName, SendEventName, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}
}

static void TS_Think( lua_State* L, idEntity* ent )
{
	// Get the register table
	lua_getglobal( L, ModuleName );
	if( !lua_istable( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find the %s table", ModuleName );
		return;
	}

	lua_getfield( L, -1, ThinkName );
	gameLocal.scriptManager.ReturnEntity( ent );

	if( lua_pcall( L, 1, 0, 0 ) != LUA_OK )
	{
		gameLocal.Warning( "Something went wrong in the %s.%s: %s", ModuleName, ThinkName, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}
}

spStateScript::spStateScript( idEntity* owner )
	: owner( owner )
{
}

void spStateScript::SetName( const char* name )
{
	scriptName = name;
}

void spStateScript::Construct()
{
	if( scriptName.IsEmpty() )
	{
		return;
	}

	gameLocal.scriptManager.AddReloadable( this );

	lua_State* L = gameLocal.scriptManager.LuaState();
	TS_New( L, owner );
}

void spStateScript::Destroy()
{
	if( scriptName.IsEmpty() )
	{
		return;
	}

	gameLocal.scriptManager.DestroyReloadable( this );

	lua_State* L = gameLocal.scriptManager.LuaState();

	lua_getglobal( L, owner->GetName() );
	if( lua_istable( L, -1 ) )
	{
		lua_pushnil( L );
		lua_setglobal( L, owner->GetName() );
	}
	lua_pop( L, 1 );
}

void spStateScript::Think()
{
	if( scriptName.IsEmpty() )
	{
		return;
	}

	lua_State* L = gameLocal.scriptManager.LuaState();

	TS_Think( L, owner );
}

void spStateScript::Reload()
{
	if( scriptName.IsEmpty() )
	{
		return;
	}

	Construct();
}

void spStateScript::SendEvent( int entityNumber, const char* eventName )
{
	if( scriptName.IsEmpty() )
	{
		return;
	}

	lua_State* L = gameLocal.scriptManager.LuaState();

	TS_SendEvent( L, owner, eventName );
}

const char* spStateScript::GetName() const
{
	return scriptName.c_str();
}

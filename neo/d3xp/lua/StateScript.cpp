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
static const char* moduleName( "EntitySystem" );
static const char* registerName( "Register" );
static const char* thinkName( "Think" );
static const char* sendEventName( "SendEvent" );
static const char* updateVariableName( "UpdateVariable" );
static const char* stateVariableName( "stateEntity" );

spStateScript::spStateScript( idEntity* owner )
	: owner( owner )
{
}

spStateScript::~spStateScript()
{
	Destroy();
}

void spStateScript::SetName( const char* name )
{
	scriptName = name;
}

void spStateScript::Construct()
{
	if( scriptName.IsEmpty() )
	{
		gameLocal.Warning( "Missing script name for state script for %s", owner->GetName() );
		return;
	}

	gameLocal.scriptManager.AddReloadable( this );

	lua_State* L = gameLocal.scriptManager.LuaState();

	// Get the register table
	lua_getglobal( L, moduleName );
	if( !lua_istable( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find the %s table", moduleName );
		return;
	}

	lua_getfield( L, -1, registerName );
	if( !lua_isfunction( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find function %s", registerName );
	}

	gameLocal.scriptManager.ReturnEntity( owner );

	if( lua_pcall( L, 1, 0, 0 ) != LUA_OK )
	{
		gameLocal.Warning( "Something went wrong in the %s.%s: %s", moduleName, registerName, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}
}

void spStateScript::Destroy()
{
	gameLocal.scriptManager.DestroyReloadable( this );

	if( scriptName.IsEmpty() )
	{
		return;
	}

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

	// Get the register table
	lua_getglobal( L, moduleName );
	if( !lua_istable( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find the %s table", moduleName );
		return;
	}

	lua_getfield( L, -1, thinkName );
	gameLocal.scriptManager.ReturnEntity( owner );

	if( lua_pcall( L, 1, 0, 0 ) != LUA_OK )
	{
		gameLocal.Warning( "Something went wrong in the %s.%s: %s", moduleName, thinkName, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}
}

void spStateScript::Reload()
{
	if( scriptName.IsEmpty() )
	{
		return;
	}

	Construct();
}

void spStateScript::SendEvent( const char* eventName )
{
	if( scriptName.IsEmpty() )
	{
		return;
	}

	lua_State* L = gameLocal.scriptManager.LuaState();

	// Get the register table
	lua_getglobal( L, moduleName );
	if( !lua_istable( L, -1 ) )
	{
		gameLocal.Warning( "Failed to find the %s table", moduleName );
		return;
	}

	lua_getfield( L, -1, sendEventName );
	gameLocal.scriptManager.ReturnEntity( owner );

	lua_createtable( L, 0, 1 );
	lua_pushstring( L, eventName );
	lua_setfield( L, -2, "name" );

	if( lua_pcall( L, 2, 0, 0 ) != LUA_OK )
	{
		gameLocal.Warning( "Something went wrong in the %s.%s: %s", moduleName, sendEventName, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}
}

void spStateScript::UpdateVariable( const char* varName, bool value ) const
{
	lua_State* L = gameLocal.scriptManager.LuaState();

	lua_getglobal( L, owner->GetName() );
	if( !lua_istable( L, -1 ) )
	{
		//gameLocal.Warning( "Failed to find registered entity %s", owner->GetName() );
		return;
	}
	lua_getfield( L, -1, stateVariableName );
	lua_pushboolean( L, value );
	lua_setfield( L, -2, varName );
	lua_pop( L, 2 );
}

const char* spStateScript::GetName() const
{
	return scriptName;
}

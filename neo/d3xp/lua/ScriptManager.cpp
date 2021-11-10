/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2021 Stephen Pridham

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

#pragma hdrstop
#include "precompiled.h"

#include "ScriptManager.h"

#include "../Game_local.h"


// TODO(Stephen): Replace this with Id's allocator system.
static void* l_alloc( void* ud, void* ptr, size_t osize, size_t nsize )
{
	( void )ud;
	( void )osize; /* not used */
	if( nsize == 0 )
	{
		free( ptr );
		return NULL;
	}
	else
	{
		return realloc( ptr, nsize );
	}
}

static int l_commonPrintf( lua_State* L )
{
	// get argument
	const char* str = lua_tostring( L, -1 );
	common->Printf( str );
	return 0;
}

static int SetLuaSearchPath( lua_State* L, const char* path )
{
	lua_getglobal( L, "package" );
	lua_getfield( L, -1, "path" );
	idStr currPath = lua_tostring( L, -1 );
	currPath.Append( ";" );
	currPath.Append( path );
	currPath.Append( "\\?.lua" );
	lua_pop( L, 1 );
	lua_pushstring( L, currPath.c_str() );
	lua_setfield( L, -2, "path" );
	lua_pop( L, 1 );
	return 0;
}

ScriptManager::ScriptManager()
	: luaState( nullptr )
{
}

ScriptManager::~ScriptManager()
{
}

bool ScriptManager::Init()
{
	InitLuaState();

	if( !LoadLuaScript( "script/main.lua" ) )
	{
		return false;
	}

	lua_getglobal( luaState, "main" );
	if( lua_pcall( luaState, 0, 0, 0 ) != LUA_OK )
	{
		gameLocal.Error( "Error running function 'main': %s\n", lua_tostring( luaState, -1 ) );
		lua_pop( luaState, 1 );
	}
	lua_pop( luaState, 1 );

	return true;
}

void ScriptManager::Shutdown()
{
	if( luaState )
	{
		lua_close( luaState );
	}
}

void ScriptManager::InitLuaState()
{
	// Set up the lua state for the game thread.
	luaState = lua_newstate( l_alloc, nullptr );
	luaL_openlibs( luaState );

	// Load the game functions into the lua state.
	lua_pushcfunction( luaState, l_commonPrintf );
	lua_setglobal( luaState, "comPrintf" );

	// Set up the search path to base/script for 'require'
	const char* scriptDir = fileSystem->RelativePathToOSPath( "script", "fs_basepath" );
	SetLuaSearchPath( luaState, scriptDir );
}

bool ScriptManager::LoadLuaScript( const char* luaScript, bool failIfNotFound )
{
	char* src;

	int length = fileSystem->ReadFile( luaScript, ( void** )&src, NULL );
	if( length < 0 )
	{
		if( failIfNotFound )
		{
			gameLocal.Error( "Failed to find lua script %s\n", luaScript );
		}
		else
		{
			gameLocal.Warning( "Failed to find lua script %s\n", luaScript );
		}

		return false;
	}

	if( luaL_loadbuffer( luaState, src, length, luaScript ) != LUA_OK || lua_pcall( luaState, 0, 0, 0 ) )
	{
		gameLocal.Warning( "Failed to load lua script %s : %s\n", luaScript, lua_tostring( luaState, -1 ) );
		lua_pop( luaState, 1 );
		fileSystem->FreeFile( src );
		return false;
	}

	fileSystem->FreeFile( src );
	return true;
}

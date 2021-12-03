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

#include "precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

#include <lua.hpp>

#include "ScriptManager.h"

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
	common->Printf( "%s", str );
	return 0;
}

static int l_initWaitingSupport( lua_State* L, double currentTime )
{
	lua_getglobal( L, "id4" );
	lua_getfield( L, -1, "Init" );
	lua_pushnumber( L, currentTime );
	int error = lua_pcall( L, 1, 0, 0 );
	if( error )
	{
		gameLocal.Error( "%s\n", lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
		return 1;
	}
	return 0;
}

// Delta time in seconds. 
static int l_wakeUpWaitingSeconds( lua_State* L, double deltaTime )
{
	lua_getglobal( L, "id4" );
	lua_getfield( L, -1, "WakeUpWaitingThreads" );
	lua_pushnumber( L, deltaTime * 0.001 );
	int error = lua_pcall( L, 1, 0, 0 );
	if( error )
	{
		gameLocal.Error("%s\n", lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
		return 1;
	}
	// pop table from stack
	lua_pop( L, 1 );
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
	: luaThread(nullptr)
{
}

ScriptManager::~ScriptManager()
{
}

bool ScriptManager::Init()
{
	startTime = gameLocal.time;

	luaThread = new LuaThread( );

	luaThread->Init( );
	
	// Add remaining support systems.
	l_initWaitingSupport( luaThread->LuaState(), (double)gameLocal.time * 0.001 );
	
	return true;
}

void ScriptManager::Shutdown()
{
	delete luaThread;
}

void ScriptManager::Reload( )
{
	// Right now it's just a hard reload.
	Shutdown( );
	Init( );
}

void ScriptManager::Restart( )
{
	//luaThread->Restart( );
}

void ScriptManager::SendEvent( const char* eventName )
{
	lua_State* L = luaThread->LuaState( );

	lua_getglobal( L, "ReceiveEvent" );

	lua_createtable( L, 1, 0 );
	lua_pushlightuserdata( L, ( void* )this );
	lua_setfield( L, -2, "classPtr" );

	lua_pushstring( L, eventName );

	if( lua_pcall( L, 2, 0, 0 ) != LUA_OK )
	{
		gameLocal.Error( "Error running function '%s': %s\n", eventName, lua_tostring( L, -1 ) );
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}

void ScriptManager::LoadScript( const char* script )
{
	luaThread->LoadLuaScript( script, true );
}

void ScriptManager::Think( int mSeconds )
{
	if( mSeconds < startTime )
	{
		startTime = mSeconds;
	}

	int deltaMs = mSeconds - startTime;
	if (firstThink)
	{
		deltaMs = 0;
	}
	l_wakeUpWaitingSeconds( luaThread->LuaState(), deltaMs );
	firstThink = false;
	startTime = mSeconds;
}

void ScriptManager::ReturnString( const char* text )
{
	luaThread->ReturnString( text );
}

void ScriptManager::Call( const char* name, int numargs )
{
	luaThread->Call( name, numargs );
}

void ScriptManager::ReturnFloat( float value )
{
	luaThread->ReturnFloat( value );
}

void ScriptManager::ReturnInt( int value )
{
	luaThread->ReturnInt( value );
}

void ScriptManager::ReturnVector( idVec3 const& vec )
{
	luaThread->ReturnVector( vec );
}

void ScriptManager::ReturnEntity( idEntity* ent )
{
	luaThread->ReturnEntity( ent );
}

const idEventDef EV_LuaThread_Print( "print", "s" );
const idEventDef EV_LuaThread_GetEntity( "getEntity", "s", 'e' );
const idEventDef EV_LuaThread_DrawText( "drawText", "svfvdf" );

CLASS_DECLARATION( idClass, LuaThread )
EVENT( EV_LuaThread_Print, LuaThread::Event_Print )
EVENT( EV_LuaThread_GetEntity, LuaThread::Event_GetEntity )
EVENT( EV_LuaThread_DrawText, LuaThread::Event_DrawText )
END_CLASS

void LuaThread::Event_Print( const char* text )
{
	gameLocal.Printf( text );
	lua_pushnil( luaState );
}

void LuaThread::Event_GetEntity( const char* name )
{
	int			entnum;
	idEntity*	ent;

	assert( name );

	if( name[0] == '*' )
	{
		entnum = atoi( &name[1] );
		if( ( entnum < 0 ) || ( entnum >= MAX_GENTITIES ) )
		{
			gameLocal.Error( "Entity number in string out of range." );
			return;
		}

		ent = gameLocal.entities[entnum];
	}
	else
	{
		ent = gameLocal.FindEntity( name );
	}

	ReturnEntity( ent );
}

void LuaThread::Event_DrawText( const char* text, const idVec3& origin, float scale, const idVec3& color, const int align, const float lifetime )
{
	gameRenderWorld->DebugText( text, origin, scale, idVec4( color.x, color.y, color.z, 1.0f ), gameLocal.GetLocalPlayer( )->viewAngles.ToMat3( ), align, SEC2MS( lifetime ) );
}

LuaThread::LuaThread( )
	: luaState( nullptr )
{
}

LuaThread::LuaThread( idEntity* self )
{
	// TODO(Stephen): Use coroutine instead?
	luaState = lua_newstate( l_alloc, nullptr );
	luaL_openlibs( luaState );
	Restart( );
}

LuaThread::~LuaThread( )
{
	lua_close( luaState );
}

void LuaThread::InitLuaState( )
{
	// Load the game functions into the lua state.
	lua_pushcfunction( luaState, l_commonPrintf );
	lua_setglobal( luaState, "comPrintf" );

	idClass::ExportLuaFunctions( luaState );

	// Set up the sys object with the metatable of this class.
	lua_createtable( luaState, 1, 0 );
	lua_pushlightuserdata( luaState, ( void* )this );
	lua_setfield( luaState, -2, "classPtr" );

	// Set up the metatable for this object.
	if( luaL_getmetatable( luaState, "LuaThread" ) == LUA_TTABLE )
	{
		lua_setmetatable( luaState, -2 );
		lua_setglobal( luaState, "sys" );
	}
	else
	{
		common->Error( "Failed to initialize metatable for LuaThread" );
		lua_pop( luaState, 1 ); // pop the table.
	}

	// Set up the search path to base/script for 'require'
	const char* scriptDir = fileSystem->RelativePathToOSPath( "script", "fs_basepath" );
	SetLuaSearchPath( luaState, scriptDir );
}

void LuaThread::Restart( )
{
	if( !LoadLuaScript( "script/scheduler.lua" ) ||
		!LoadLuaScript( "script/state.lua" ) ||
		!LoadLuaScript( "script/main.lua" ) )
	{
		gameLocal.Error("Failed to load lua scripts");
	}

	lua_getglobal( luaState, "main" );
	if( lua_pcall( luaState, 0, 0, 0 ) != LUA_OK )
	{
		gameLocal.Error( "Error running function 'main': %s\n", lua_tostring( luaState, -1 ) );
		lua_pop( luaState, 1 );
	}
	lua_pop( luaState, 1 );
}

bool LuaThread::LoadLuaScript( const char* luaScript, bool failIfNotFound )
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

	if( luaL_loadbuffer( luaState, src, length, luaScript ) != LUA_OK || lua_pcall( luaState, 0, 0, 0 ) != LUA_OK )
	{
		gameLocal.Warning( "Failed to load lua script %s : %s\n", luaScript, lua_tostring( luaState, -1 ) );
		lua_pop( luaState, 1 );
		fileSystem->FreeFile( src );
		return false;
	}

	fileSystem->FreeFile( src );
	return true;
}

void LuaThread::Init( )
{
	// Set up the lua state for the game thread.
	luaState = lua_newstate( l_alloc, nullptr );
	luaL_openlibs( luaState );

	InitLuaState( );
	Restart( );
}

void LuaThread::ReturnString( const char* text )
{
	lua_pushstring( luaState, text );
}

void LuaThread::ReturnFloat( float value )
{
	lua_pushnumber( luaState, value );
}

void LuaThread::Call( const char* name, int numargs )
{
	lua_getglobal( luaState, name );
	lua_pcall( luaState, numargs, 0, 0 );
}

void LuaThread::ReturnInt( int value )
{
	lua_pushinteger( luaState, value );
}

void LuaThread::ReturnVector( idVec3 const& vec )
{
	lua_createtable( luaState, 0, 6 );
	lua_pushnumber( luaState, vec.x );
	lua_setfield( luaState, -2, "x" );
	lua_pushnumber( luaState, vec.y );
	lua_setfield( luaState, -2, "y" );
	lua_pushnumber( luaState, vec.z );
	lua_setfield( luaState, -2, "z" );
}

void LuaThread::ReturnEntity( idEntity* ent )
{
	if( !ent )
	{
		lua_pushnil( luaState );
		return;
	}

	lua_createtable( luaState, 0, 2 );
	lua_pushlightuserdata( luaState, ( void* )ent );
	lua_setfield( luaState, -2, "classPtr" );

	if( luaL_getmetatable( luaState, ent->GetClassname( ) ) == LUA_TTABLE )
	{
		lua_setmetatable( luaState, -2 );
	}
	else
	{
		common->Error( "Failed to initialize metatable for %s\n", ent->GetClassname( ) );
	}
}

ScopedLuaState::ScopedLuaState( lua_State* L )
	: originalThread( gameLocal.scriptManager.luaThread->luaState )
	, newThread( L )
{
	gameLocal.scriptManager.luaThread->luaState = newThread;
}

ScopedLuaState::~ScopedLuaState( )
{
	// Restore the original state.
	gameLocal.scriptManager.luaThread->luaState = originalThread;
}
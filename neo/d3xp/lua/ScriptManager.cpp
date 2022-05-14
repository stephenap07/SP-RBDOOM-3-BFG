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

const idEventDef EV_LuaThread_TerminateThread( "terminate", "d" );
const idEventDef EV_LuaThread_Pause( "pause", NULL );
const idEventDef EV_LuaThread_WaitFor( "waitFor", "e" );
const idEventDef EV_LuaThread_WaitForThread( "waitForThread", "d" );
const idEventDef EV_LuaThread_Print( "print", "s" );
const idEventDef EV_LuaThread_PrintLn( "println", "s" );
const idEventDef EV_LuaThread_Say( "say", "s" );
const idEventDef EV_LuaThread_Assert( "assert", "f" );
const idEventDef EV_LuaThread_Trigger( "trigger", "e" );
const idEventDef EV_LuaThread_SetCvar( "setcvar", "ss" );
const idEventDef EV_LuaThread_GetCvar( "getcvar", "s", 's' );
const idEventDef EV_LuaThread_Random( "random", "f", 'f' );
const idEventDef EV_LuaThread_RandomInt( "randomInt", "d", 'd' );
const idEventDef EV_LuaThread_GetTime( "getTime", NULL, 'f' );
const idEventDef EV_LuaThread_KillThread( "killthread", "s" );
const idEventDef EV_LuaThread_SetThreadName( "threadname", "s" );
const idEventDef EV_LuaThread_GetEntity( "getEntity", "s", 'e' );
const idEventDef EV_LuaThread_Spawn( "spawn", "s", 'e' );
const idEventDef EV_LuaThread_CopySpawnArgs( "copySpawnArgs", "e" );
const idEventDef EV_LuaThread_SetSpawnArg( "setSpawnArg", "ss" );
const idEventDef EV_LuaThread_SpawnString( "SpawnString", "ss", 's' );
const idEventDef EV_LuaThread_SpawnFloat( "SpawnFloat", "sf", 'f' );
const idEventDef EV_LuaThread_SpawnVector( "SpawnVector", "sv", 'v' );
const idEventDef EV_LuaThread_ClearPersistantArgs( "clearPersistantArgs" );
const idEventDef EV_LuaThread_SetPersistantArg( "setPersistantArg", "ss" );
const idEventDef EV_LuaThread_GetPersistantString( "getPersistantString", "s", 's' );
const idEventDef EV_LuaThread_GetPersistantFloat( "getPersistantFloat", "s", 'f' );
const idEventDef EV_LuaThread_GetPersistantVector( "getPersistantVector", "s", 'v' );
const idEventDef EV_LuaThread_AngToForward( "angToForward", "v", 'v' );
const idEventDef EV_LuaThread_AngToRight( "angToRight", "v", 'v' );
const idEventDef EV_LuaThread_AngToUp( "angToUp", "v", 'v' );
const idEventDef EV_LuaThread_Sine( "sin", "f", 'f' );
const idEventDef EV_LuaThread_Cosine( "cos", "f", 'f' );
const idEventDef EV_LuaThread_ArcSine( "asin", "f", 'f' );
const idEventDef EV_LuaThread_ArcCosine( "acos", "f", 'f' );
const idEventDef EV_LuaThread_SquareRoot( "sqrt", "f", 'f' );
const idEventDef EV_LuaThread_Normalize( "vecNormalize", "v", 'v' );
const idEventDef EV_LuaThread_VecLength( "vecLength", "v", 'f' );
const idEventDef EV_LuaThread_VecDotProduct( "DotProduct", "vv", 'f' );
const idEventDef EV_LuaThread_VecCrossProduct( "CrossProduct", "vv", 'v' );
const idEventDef EV_LuaThread_VecToAngles( "VecToAngles", "v", 'v' );
const idEventDef EV_LuaThread_VecToOrthoBasisAngles( "VecToOrthoBasisAngles", "v", 'v' );
const idEventDef EV_LuaThread_RotateVector( "rotateVector", "vv", 'v' );
const idEventDef EV_LuaThread_OnSignal( "onSignal", "des" );
const idEventDef EV_LuaThread_ClearSignal( "clearSignalThread", "de" );
const idEventDef EV_LuaThread_SetCamera( "setCamera", "e" );
const idEventDef EV_LuaThread_FirstPerson( "firstPerson", NULL );
const idEventDef EV_LuaThread_Trace( "trace", "vvvvde", 'f' );
const idEventDef EV_LuaThread_TracePoint( "tracePoint", "vvde", 'f' );
const idEventDef EV_LuaThread_GetTraceFraction( "getTraceFraction", NULL, 'f' );
const idEventDef EV_LuaThread_GetTraceEndPos( "getTraceEndPos", NULL, 'v' );
const idEventDef EV_LuaThread_GetTraceNormal( "getTraceNormal", NULL, 'v' );
const idEventDef EV_LuaThread_GetTraceEntity( "getTraceEntity", NULL, 'e' );
const idEventDef EV_LuaThread_GetTraceJoint( "getTraceJoint", NULL, 's' );
const idEventDef EV_LuaThread_GetTraceBody( "getTraceBody", NULL, 's' );
const idEventDef EV_LuaThread_FadeIn( "fadeIn", "vf" );
const idEventDef EV_LuaThread_FadeOut( "fadeOut", "vf" );
const idEventDef EV_LuaThread_FadeTo( "fadeTo", "vff" );
const idEventDef EV_LuaThread_StartMusic( "music", "s" );
const idEventDef EV_LuaThread_Error( "error", "s" );
const idEventDef EV_LuaThread_Warning( "warning", "s" );
const idEventDef EV_LuaThread_StrLen( "strLength", "s", 'd' );
const idEventDef EV_LuaThread_StrLeft( "strLeft", "sd", 's' );
const idEventDef EV_LuaThread_StrRight( "strRight", "sd", 's' );
const idEventDef EV_LuaThread_StrSkip( "strSkip", "sd", 's' );
const idEventDef EV_LuaThread_StrMid( "strMid", "sdd", 's' );
const idEventDef EV_LuaThread_StrToFloat( "strToFloat", "s", 'f' );
const idEventDef EV_LuaThread_RadiusDamage( "radiusDamage", "vEEEsf" );
const idEventDef EV_LuaThread_IsClient( "isClient", NULL, 'f' );
const idEventDef EV_LuaThread_IsMultiplayer( "isMultiplayer", NULL, 'f' );
const idEventDef EV_LuaThread_GetFrameTime( "getFrameTime", NULL, 'f' );
const idEventDef EV_LuaThread_GetTicsPerSecond( "getTicsPerSecond", NULL, 'f' );
const idEventDef EV_LuaThread_DebugLine( "debugLine", "vvvf" );
const idEventDef EV_LuaThread_DebugArrow( "debugArrow", "vvvdf" );
const idEventDef EV_LuaThread_DebugCircle( "debugCircle", "vvvfdf" );
const idEventDef EV_LuaThread_DebugBounds( "debugBounds", "vvvf" );
const idEventDef EV_LuaThread_DrawText( "drawText", "svfvdf" );
const idEventDef EV_LuaThread_InfluenceActive( "influenceActive", NULL, 'd' );

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
		gameLocal.Error( "%s\n", lua_tostring( L, -1 ) );
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
	: luaThread( nullptr )
{
}

ScriptManager::~ScriptManager()
{
}

bool ScriptManager::Init()
{
	luaThread = new idLuaThread();

	luaThread->Init( );

	return true;
}

void ScriptManager::Shutdown()
{
	delete luaThread;
    luaThread = nullptr;
}

void ScriptManager::Reload( )
{
	Shutdown();
	Init();

	for( int i = 0; i < reloadables.Num(); i++ )
	{
		reloadables[i]->Reload();
	}
}

void ScriptManager::Restart( )
{
	luaThread->Restart( );
}

void ScriptManager::LoadScript( const char* script )
{
	luaThread->LoadLuaScript( script, true );
}

void ScriptManager::AddReloadable( idStateScript* stateScript )
{
	reloadables.AddUnique( stateScript );
}

void ScriptManager::DestroyReloadable( idStateScript* stateScript )
{
	reloadables.Remove( stateScript );
}

void ScriptManager::ReturnString( const char* text )
{
	luaThread->ReturnString( text );
}

void ScriptManager::Call_f( const idCmdArgs& args )
{
	gameLocal.scriptManager.luaThread->Call( args );
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

/*
================
idThread::Error
================
*/
void ScriptManager::Error( const char* fmt, ... ) const
{
	char text[MAX_STRING_CHARS];
	va_list ap;

	va_start( ap, fmt );
	vsprintf( text, fmt, ap );
	va_end( ap );

	idLib::common->Error( text );
}

/*
================
idThread::Warning
================
*/
void ScriptManager::Warning( const char* fmt, ... ) const
{
	char text[MAX_STRING_CHARS];
	va_list ap;

	va_start( ap, fmt );
	vsprintf( text, fmt, ap );
	va_end( ap );

	idLib::common->Warning( text );
}

trace_t idLuaThread::trace;

CLASS_DECLARATION( idClass, idLuaThread )
EVENT( EV_LuaThread_Print, idLuaThread::Event_Print )
EVENT( EV_LuaThread_PrintLn, idLuaThread::Event_PrintLn )
EVENT( EV_LuaThread_Say, idLuaThread::Event_Say )
EVENT( EV_LuaThread_Assert, idLuaThread::Event_Assert )
EVENT( EV_LuaThread_Trigger, idLuaThread::Event_Trigger )
EVENT( EV_LuaThread_SetCvar, idLuaThread::Event_SetCvar )
EVENT( EV_LuaThread_GetCvar, idLuaThread::Event_GetCvar )
EVENT( EV_LuaThread_Random, idLuaThread::Event_Random )
EVENT( EV_LuaThread_RandomInt, idLuaThread::Event_RandomInt )
EVENT( EV_LuaThread_GetTime, idLuaThread::Event_GetTime )
EVENT( EV_LuaThread_GetEntity, idLuaThread::Event_GetEntity )
EVENT( EV_LuaThread_Spawn, idLuaThread::Event_Spawn )
EVENT( EV_LuaThread_CopySpawnArgs, idLuaThread::Event_CopySpawnArgs )
EVENT( EV_LuaThread_SetSpawnArg, idLuaThread::Event_SetSpawnArg )
EVENT( EV_LuaThread_SpawnString, idLuaThread::Event_SpawnString )
EVENT( EV_LuaThread_SpawnFloat, idLuaThread::Event_SpawnFloat )
EVENT( EV_LuaThread_SpawnVector, idLuaThread::Event_SpawnVector )
EVENT( EV_LuaThread_ClearPersistantArgs, idLuaThread::Event_ClearPersistantArgs )
EVENT( EV_LuaThread_SetPersistantArg, idLuaThread::Event_SetPersistantArg )
EVENT( EV_LuaThread_GetPersistantString, idLuaThread::Event_GetPersistantString )
EVENT( EV_LuaThread_GetPersistantFloat, idLuaThread::Event_GetPersistantFloat )
EVENT( EV_LuaThread_GetPersistantVector, idLuaThread::Event_GetPersistantVector )
EVENT( EV_LuaThread_AngToForward, idLuaThread::Event_AngToForward )
EVENT( EV_LuaThread_AngToRight, idLuaThread::Event_AngToRight )
EVENT( EV_LuaThread_AngToUp, idLuaThread::Event_AngToUp )
EVENT( EV_LuaThread_Sine, idLuaThread::Event_GetSine )
EVENT( EV_LuaThread_Cosine, idLuaThread::Event_GetCosine )
EVENT( EV_LuaThread_ArcSine, idLuaThread::Event_GetArcSine )
EVENT( EV_LuaThread_ArcCosine, idLuaThread::Event_GetArcCosine )
EVENT( EV_LuaThread_SquareRoot, idLuaThread::Event_GetSquareRoot )
EVENT( EV_LuaThread_Normalize, idLuaThread::Event_VecNormalize )
EVENT( EV_LuaThread_VecLength, idLuaThread::Event_VecLength )
EVENT( EV_LuaThread_VecDotProduct, idLuaThread::Event_VecDotProduct )
EVENT( EV_LuaThread_VecCrossProduct, idLuaThread::Event_VecCrossProduct )
EVENT( EV_LuaThread_VecToAngles, idLuaThread::Event_VecToAngles )
EVENT( EV_LuaThread_VecToOrthoBasisAngles, idLuaThread::Event_VecToOrthoBasisAngles )
EVENT( EV_LuaThread_RotateVector, idLuaThread::Event_RotateVector )
EVENT( EV_LuaThread_OnSignal, idLuaThread::Event_OnSignal )
EVENT( EV_LuaThread_ClearSignal, idLuaThread::Event_ClearSignalThread )
EVENT( EV_LuaThread_SetCamera, idLuaThread::Event_SetCamera )
EVENT( EV_LuaThread_FirstPerson, idLuaThread::Event_FirstPerson )
EVENT( EV_LuaThread_Trace, idLuaThread::Event_Trace )
EVENT( EV_LuaThread_TracePoint, idLuaThread::Event_TracePoint )
EVENT( EV_LuaThread_GetTraceFraction, idLuaThread::Event_GetTraceFraction )
EVENT( EV_LuaThread_GetTraceEndPos, idLuaThread::Event_GetTraceEndPos )
EVENT( EV_LuaThread_GetTraceNormal, idLuaThread::Event_GetTraceNormal )
EVENT( EV_LuaThread_GetTraceEntity, idLuaThread::Event_GetTraceEntity )
EVENT( EV_LuaThread_GetTraceJoint, idLuaThread::Event_GetTraceJoint )
EVENT( EV_LuaThread_GetTraceBody, idLuaThread::Event_GetTraceBody )
EVENT( EV_LuaThread_FadeIn, idLuaThread::Event_FadeIn )
EVENT( EV_LuaThread_FadeOut, idLuaThread::Event_FadeOut )
EVENT( EV_LuaThread_FadeTo, idLuaThread::Event_FadeTo )
EVENT( EV_SetShaderParm, idLuaThread::Event_SetShaderParm )
EVENT( EV_LuaThread_StartMusic, idLuaThread::Event_StartMusic )
EVENT( EV_LuaThread_Warning, idLuaThread::Event_Warning )
EVENT( EV_LuaThread_Error, idLuaThread::Event_Error )
EVENT( EV_LuaThread_StrLen, idLuaThread::Event_StrLen )
EVENT( EV_LuaThread_StrLeft, idLuaThread::Event_StrLeft )
EVENT( EV_LuaThread_StrRight, idLuaThread::Event_StrRight )
EVENT( EV_LuaThread_StrSkip, idLuaThread::Event_StrSkip )
EVENT( EV_LuaThread_StrMid, idLuaThread::Event_StrMid )
EVENT( EV_LuaThread_StrToFloat, idLuaThread::Event_StrToFloat )
EVENT( EV_LuaThread_RadiusDamage, idLuaThread::Event_RadiusDamage )
EVENT( EV_LuaThread_IsClient, idLuaThread::Event_IsClient )
EVENT( EV_LuaThread_IsMultiplayer, idLuaThread::Event_IsMultiplayer )
EVENT( EV_LuaThread_GetFrameTime, idLuaThread::Event_GetFrameTime )
EVENT( EV_LuaThread_GetTicsPerSecond, idLuaThread::Event_GetTicsPerSecond )
EVENT( EV_CacheSoundShader, idLuaThread::Event_CacheSoundShader )
EVENT( EV_LuaThread_DebugLine, idLuaThread::Event_DebugLine )
EVENT( EV_LuaThread_DebugArrow, idLuaThread::Event_DebugArrow )
EVENT( EV_LuaThread_DebugCircle, idLuaThread::Event_DebugCircle )
EVENT( EV_LuaThread_DebugBounds, idLuaThread::Event_DebugBounds )
EVENT( EV_LuaThread_DrawText, idLuaThread::Event_DrawText )
EVENT( EV_LuaThread_InfluenceActive, idLuaThread::Event_InfluenceActive )
END_CLASS

idLuaThread::idLuaThread( )
	: luaState( nullptr )
{
}

idLuaThread::idLuaThread( idEntity* self )
{
	// TODO(Stephen): Use coroutine instead?
	luaState = lua_newstate( l_alloc, nullptr );
	luaL_openlibs( luaState );
	Restart( );
}

idLuaThread::~idLuaThread( )
{
	lua_close( luaState );
}

void idLuaThread::InitLuaState( )
{
	// Load the game functions into the lua state.
	idClass::ExportLuaFunctions( luaState );

	// Set up the search path to base/script for 'require'
	const char* scriptDir = fileSystem->RelativePathToOSPath( "script", "fs_basepath" );
	SetLuaSearchPath( luaState, scriptDir );
	cmdSystem->AddCommand( "doLua", ScriptManager::Call_f, CMD_FL_GAME | CMD_FL_CHEAT, "executes some arbitrary lua" );

	// Set up the sys object with the metatable of this class.
	lua_State* L = luaState;

	constexpr const char* threadName = "sys";

	lua_getglobal( L, threadName );
	if( lua_istable( L, -1 ) )
	{
		return;
	}
	lua_pop( L, 1 );

	// Create the instance table
	lua_createtable( L, 0, 2 );
	lua_pushlightuserdata( L, this );
	lua_setfield( L, -2, "classPtr" );

	// Create the metatable
	lua_createtable( L, 0, 1 );
	lua_getglobal( L, GetClassname() );
	if( !lua_istable( L, -1 ) )
	{
		common->FatalError( "Failed to get table for %s", GetClassname() );
		lua_pop( L, 1 );
		return;
	}
	lua_setfield( L, -2, "__index" );
	lua_setmetatable( L, -2 );

	// _G[TheEntityName] = this new class instance.
	lua_setglobal( L, threadName );

	// push the table back onto the stack.
	lua_getglobal( L, threadName );
}

void idLuaThread::Restart( )
{
	if( !LoadLuaScript( "script/entry.lua" ) )
	{
		gameLocal.Error( "Failed to load lua scripts" );
	}

	lua_getglobal( luaState, "tech4" );
	if( !lua_istable( luaState, -1 ) )
	{
		common->FatalError( "Failed to define tech4 module" );
		lua_pop( luaState, 1 );
		return;
	}

	lua_getfield( luaState, -1, "main" );

	if( lua_pcall( luaState, 0, 0, 0 ) != LUA_OK )
	{
		common->FatalError( "Error running function 'main': %s\n", lua_tostring( luaState, -1 ) );
		lua_pop( luaState, 1 );
		return;
	}

	lua_pop( luaState, 1 );
}

bool idLuaThread::LoadLuaScript( const char* luaScript, bool failIfNotFound )
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

void idLuaThread::Init( )
{
	// Set up the lua state for the game thread.
	luaState = lua_newstate( l_alloc, nullptr );
	luaL_openlibs( luaState );

	InitLuaState( );
	Restart( );
}

void idLuaThread::ReturnString( const char* text ) const
{
	lua_pushstring( luaState, text );
}

void idLuaThread::ReturnFloat( float value ) const
{
	lua_pushnumber( luaState, value );
}

void idLuaThread::Call( const idCmdArgs& args ) const
{
	lua_getglobal( luaState, "loadstring" );
	lua_pushstring( luaState, args.Args() );
	lua_pcall( luaState, 1, 0, 0 );
}

void idLuaThread::ReturnInt( int value ) const
{
	lua_pushinteger( luaState, value );
}

void idLuaThread::ReturnVector( idVec3 const& vec ) const
{
	// TODO(Stephen): Test nrecs = 3 instead of 6.
	lua_createtable( luaState, 0, 6 );
	lua_pushnumber( luaState, vec.x );
	lua_setfield( luaState, -2, "x" );
	lua_pushnumber( luaState, vec.y );
	lua_setfield( luaState, -2, "y" );
	lua_pushnumber( luaState, vec.z );
	lua_setfield( luaState, -2, "z" );
}

void idLuaThread::ReturnEntity( idEntity* ent ) const
{
	lua_State* L = luaState;

	if( !ent )
	{
		lua_pushnil( L );
		return;
	}

	lua_getglobal( L, ent->GetName() );
	if( lua_istable( L, -1 ) )
	{
		return;
	}
	lua_pop( L, 1 );

	// Create the instance table
	lua_createtable( L, 0, 2 );
	lua_pushlightuserdata( L, ( void* )ent );
	lua_setfield( L, -2, "classPtr" );

	// Create the metatable
	lua_createtable( L, 0, 1 );
	lua_getglobal( L, ent->GetClassname() );
	if( !lua_istable( L, -1 ) )
	{
		common->FatalError( "Failed to get table for %s", ent->GetClassname() );
		lua_pop( L, 1 );
		return;
	}
	lua_setfield( L, -2, "__index" );
	lua_setmetatable( L, -2 );

	// _G[TheEntityName] = this new class instance.
	lua_setglobal( L, ent->GetName() );

	// push the table back onto the stack.
	lua_getglobal( L, ent->GetName() );
}

ScopedLuaState::ScopedLuaState( lua_State* L )
	: originalProgramThread( gameLocal.program.LuaState() )
	, originalThread( gameLocal.scriptManager.luaThread->luaState )
	, newThread( L )
{
	gameLocal.program.SetLuaState( newThread );
	gameLocal.scriptManager.luaThread->luaState = newThread;
}

ScopedLuaState::~ScopedLuaState( )
{
	// Restore the original state.
	gameLocal.program.SetLuaState( originalProgramThread );
	gameLocal.scriptManager.luaThread->luaState = originalThread;
}

/*
================
idThread::Error
================
*/
void idLuaThread::Error( const char* fmt, ... ) const
{
	va_list	argptr;
	char	text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	// Pass error into lua

	//interpreter.Error( text );
}

/*
================
idThread::Warning
================
*/
void idLuaThread::Warning( const char* fmt, ... ) const
{
	va_list	argptr;
	char	text[1024];

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	// Pass warning into lua

	//interpreter.Warning( text );
}


/***********************************************************************

  Script callable events

***********************************************************************/

/*
================
LuaThread::Event_Print
================
*/
void idLuaThread::Event_Print( const char* text )
{
	gameLocal.Printf( "%s", text );
}

/*
================
LuaThread::Event_PrintLn
================
*/
void idLuaThread::Event_PrintLn( const char* text )
{
	gameLocal.Printf( "%s\n", text );
}

/*
================
LuaThread::Event_Say
================
*/
void idLuaThread::Event_Say( const char* text )
{
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "say \"%s\"", text ) );
}

/*
================
LuaThread::Event_Assert
================
*/
void idLuaThread::Event_Assert( float value )
{
	assert( value );
}

/*
================
LuaThread::Event_Trigger
================
*/
void idLuaThread::Event_Trigger( idEntity* ent )
{
	if( ent )
	{
		ent->Signal( SIG_TRIGGER );
		ent->ProcessEvent( &EV_Activate, gameLocal.GetLocalPlayer() );
		ent->TriggerGuis();
	}
}

/*
================
LuaThread::Event_SetCvar
================
*/
void idLuaThread::Event_SetCvar( const char* name, const char* value ) const
{
	cvarSystem->SetCVarString( name, value );
}

/*
================
LuaThread::Event_GetCvar
================
*/
void idLuaThread::Event_GetCvar( const char* name ) const
{
	ReturnString( cvarSystem->GetCVarString( name ) );
}

/*
================
LuaThread::Event_Random
================
*/
void idLuaThread::Event_Random( float range ) const
{
	float result;

	result = gameLocal.random.RandomFloat();
	ReturnFloat( range * result );
}

/*
================
LuaThread::Event_RandomInt
================
*/
void idLuaThread::Event_RandomInt( int range ) const
{
	int result;
	result = gameLocal.random.RandomInt( range );
	ReturnFloat( result );
}

/*
================
LuaThread::Event_GetTime
================
*/
void idLuaThread::Event_GetTime()
{

	ReturnFloat( MS2SEC( gameLocal.realClientTime ) );

	/*  Script always uses realClient time to determine scripty stuff. ( This Fixes Weapon Animation timing bugs )
	if ( common->IsMultiplayer() ) {
		ReturnFloat( MS2SEC( gameLocal.GetServerGameTimeMs() ) );
	} else {
		ReturnFloat( MS2SEC( gameLocal.realClientTime ) );
	}
	*/
}

/*
================
LuaThread::Event_GetEntity
================
*/
void idLuaThread::Event_GetEntity( const char* name )
{
	int			entnum;
	idEntity* ent;

	assert( name );

	if( name[0] == '*' )
	{
		entnum = atoi( &name[1] );
		if( ( entnum < 0 ) || ( entnum >= MAX_GENTITIES ) )
		{
			Error( "Entity number in string out of range." );
			return;
		}
		ReturnEntity( gameLocal.entities[entnum] );
	}
	else
	{
		ent = gameLocal.FindEntity( name );
		ReturnEntity( ent );
	}
}

/*
================
LuaThread::Event_Spawn
================
*/
void idLuaThread::Event_Spawn( const char* classname )
{
	idEntity* ent;

	spawnArgs.Set( "classname", classname );
	gameLocal.SpawnEntityDef( spawnArgs, &ent );
	ReturnEntity( ent );
	spawnArgs.Clear();
}

/*
================
LuaThread::Event_CopySpawnArgs
================
*/
void idLuaThread::Event_CopySpawnArgs( idEntity* ent )
{
	spawnArgs.Copy( ent->spawnArgs );
}

/*
================
LuaThread::Event_SetSpawnArg
================
*/
void idLuaThread::Event_SetSpawnArg( const char* key, const char* value )
{
	spawnArgs.Set( key, value );
}

/*
================
LuaThread::Event_SpawnString
================
*/
void idLuaThread::Event_SpawnString( const char* key, const char* defaultvalue )
{
	const char* result;

	spawnArgs.GetString( key, defaultvalue, &result );
	ReturnString( result );
}

/*
================
LuaThread::Event_SpawnFloat
================
*/
void idLuaThread::Event_SpawnFloat( const char* key, float defaultvalue )
{
	float result;

	spawnArgs.GetFloat( key, va( "%f", defaultvalue ), result );
	ReturnFloat( result );
}

/*
================
LuaThread::Event_SpawnVector
================
*/
void idLuaThread::Event_SpawnVector( const char* key, idVec3& defaultvalue )
{
	idVec3 result;

	spawnArgs.GetVector( key, va( "%f %f %f", defaultvalue.x, defaultvalue.y, defaultvalue.z ), result );
	ReturnVector( result );
}

/*
================
LuaThread::Event_ClearPersistantArgs
================
*/
void idLuaThread::Event_ClearPersistantArgs()
{
	gameLocal.persistentLevelInfo.Clear();
}

/*
================
LuaThread::Event_SetPersistantArg
================
*/
void idLuaThread::Event_SetPersistantArg( const char* key, const char* value )
{
	gameLocal.persistentLevelInfo.Set( key, value );
}

/*
================
LuaThread::Event_GetPersistantString
================
*/
void idLuaThread::Event_GetPersistantString( const char* key )
{
	const char* result;

	gameLocal.persistentLevelInfo.GetString( key, "", &result );
	ReturnString( result );
}

/*
================
LuaThread::Event_GetPersistantFloat
================
*/
void idLuaThread::Event_GetPersistantFloat( const char* key )
{
	float result;

	gameLocal.persistentLevelInfo.GetFloat( key, "0", result );
	ReturnFloat( result );
}

/*
================
LuaThread::Event_GetPersistantVector
================
*/
void idLuaThread::Event_GetPersistantVector( const char* key )
{
	idVec3 result;

	gameLocal.persistentLevelInfo.GetVector( key, "0 0 0", result );
	ReturnVector( result );
}

/*
================
LuaThread::Event_AngToForward
================
*/
void idLuaThread::Event_AngToForward( idAngles& ang )
{
	ReturnVector( ang.ToForward() );
}

/*
================
LuaThread::Event_AngToRight
================
*/
void idLuaThread::Event_AngToRight( idAngles& ang )
{
	idVec3 vec;

	ang.ToVectors( NULL, &vec );
	ReturnVector( vec );
}

/*
================
LuaThread::Event_AngToUp
================
*/
void idLuaThread::Event_AngToUp( idAngles& ang )
{
	idVec3 vec;

	ang.ToVectors( NULL, NULL, &vec );
	ReturnVector( vec );
}

/*
================
LuaThread::Event_GetSine
================
*/
void idLuaThread::Event_GetSine( float angle )
{
	ReturnFloat( idMath::Sin( DEG2RAD( angle ) ) );
}

/*
================
LuaThread::Event_GetCosine
================
*/
void idLuaThread::Event_GetCosine( float angle )
{
	ReturnFloat( idMath::Cos( DEG2RAD( angle ) ) );
}

/*
================
LuaThread::Event_GetArcSine
================
*/
void idLuaThread::Event_GetArcSine( float a )
{
	ReturnFloat( RAD2DEG( idMath::ASin( a ) ) );
}

/*
================
LuaThread::Event_GetArcCosine
================
*/
void idLuaThread::Event_GetArcCosine( float a )
{
	ReturnFloat( RAD2DEG( idMath::ACos( a ) ) );
}

/*
================
LuaThread::Event_GetSquareRoot
================
*/
void idLuaThread::Event_GetSquareRoot( float theSquare )
{
	ReturnFloat( idMath::Sqrt( theSquare ) );
}

/*
================
LuaThread::Event_VecNormalize
================
*/
void idLuaThread::Event_VecNormalize( idVec3& vec )
{
	idVec3 n;

	n = vec;
	n.Normalize();
	ReturnVector( n );
}

/*
================
LuaThread::Event_VecLength
================
*/
void idLuaThread::Event_VecLength( idVec3& vec )
{
	ReturnFloat( vec.Length() );
}

/*
================
LuaThread::Event_VecDotProduct
================
*/
void idLuaThread::Event_VecDotProduct( idVec3& vec1, idVec3& vec2 )
{
	ReturnFloat( vec1 * vec2 );
}

/*
================
LuaThread::Event_VecCrossProduct
================
*/
void idLuaThread::Event_VecCrossProduct( idVec3& vec1, idVec3& vec2 )
{
	ReturnVector( vec1.Cross( vec2 ) );
}

/*
================
LuaThread::Event_VecToAngles
================
*/
void idLuaThread::Event_VecToAngles( idVec3& vec )
{
	idAngles ang = vec.ToAngles();
	ReturnVector( idVec3( ang[0], ang[1], ang[2] ) );
}

/*
================
LuaThread::Event_VecToOrthoBasisAngles
================
*/
void idLuaThread::Event_VecToOrthoBasisAngles( idVec3& vec )
{
	idVec3 left, up;
	idAngles ang;

	vec.OrthogonalBasis( left, up );
	idMat3 axis( left, up, vec );

	ang = axis.ToAngles();

	ReturnVector( idVec3( ang[0], ang[1], ang[2] ) );
}

void idLuaThread::Event_RotateVector( idVec3& vec, idVec3& ang )
{

	idAngles tempAng( ang );
	idMat3 axis = tempAng.ToMat3();
	idVec3 ret = vec * axis;
	ReturnVector( ret );

}

/*
================
LuaThread::Event_OnSignal
================
*/
void idLuaThread::Event_OnSignal( int signal, idEntity* ent, const char* func )
{
	const function_t* function;

	assert( func );

	if( ent == NULL )
	{
		Error( "Entity not found" );
		return;
	}

	if( ( signal < 0 ) || ( signal >= NUM_SIGNALS ) )
	{
		Error( "Signal out of range" );
	}

	function = gameLocal.program.FindFunction( func );
	if( !function )
	{
		Error( "Function '%s' not found", func );
	}

	//ent->SetSignal( ( signalNum_t )signal, this, function );
}

/*
================
LuaThread::Event_ClearSignalThread
================
*/
void idLuaThread::Event_ClearSignalThread( int signal, idEntity* ent )
{
	if( ent == NULL )
	{
		Error( "Entity not found" );
		return;
	}

	if( ( signal < 0 ) || ( signal >= NUM_SIGNALS ) )
	{
		Error( "Signal out of range" );
	}

	//ent->ClearSignalThread( ( signalNum_t )signal, this );
}

/*
================
LuaThread::Event_SetCamera
================
*/
void idLuaThread::Event_SetCamera( idEntity* ent )
{
	if( !ent )
	{
		Error( "Entity not found" );
		return;
	}

	if( !ent->IsType( idCamera::Type ) )
	{
		Error( "Entity is not a camera" );
		return;
	}

	gameLocal.SetCamera( ( idCamera* )ent );
}

/*
================
LuaThread::Event_FirstPerson
================
*/
void idLuaThread::Event_FirstPerson()
{
	gameLocal.SetCamera( NULL );
}

/*
================
LuaThread::Event_Trace
================
*/
void idLuaThread::Event_Trace( const idVec3& start, const idVec3& end, const idVec3& mins, const idVec3& maxs, int contents_mask, idEntity* passEntity )
{
	if( mins == vec3_origin && maxs == vec3_origin )
	{
		gameLocal.clip.TracePoint( trace, start, end, contents_mask, passEntity );
	}
	else
	{
		gameLocal.clip.TraceBounds( trace, start, end, idBounds( mins, maxs ), contents_mask, passEntity );
	}
	ReturnFloat( trace.fraction );
}

/*
================
LuaThread::Event_TracePoint
================
*/
void idLuaThread::Event_TracePoint( const idVec3& start, const idVec3& end, int contents_mask, idEntity* passEntity )
{
	gameLocal.clip.TracePoint( trace, start, end, contents_mask, passEntity );
	ReturnFloat( trace.fraction );
}

/*
================
LuaThread::Event_GetTraceFraction
================
*/
void idLuaThread::Event_GetTraceFraction()
{
	ReturnFloat( trace.fraction );
}

/*
================
LuaThread::Event_GetTraceEndPos
================
*/
void idLuaThread::Event_GetTraceEndPos()
{
	ReturnVector( trace.endpos );
}

/*
================
LuaThread::Event_GetTraceNormal
================
*/
void idLuaThread::Event_GetTraceNormal()
{
	if( trace.fraction < 1.0f )
	{
		ReturnVector( trace.c.normal );
	}
	else
	{
		ReturnVector( vec3_origin );
	}
}

/*
================
LuaThread::Event_GetTraceEntity
================
*/
void idLuaThread::Event_GetTraceEntity()
{
	if( trace.fraction < 1.0f )
	{
		ReturnEntity( gameLocal.entities[trace.c.entityNum] );
	}
	else
	{
		ReturnEntity( ( idEntity* )NULL );
	}
}

/*
================
LuaThread::Event_GetTraceJoint
================
*/
void idLuaThread::Event_GetTraceJoint()
{
	if( trace.fraction < 1.0f && trace.c.id < 0 )
	{
		idAFEntity_Base* af = static_cast< idAFEntity_Base* >( gameLocal.entities[trace.c.entityNum] );
		if( af && af->IsType( idAFEntity_Base::Type ) && af->IsActiveAF() )
		{
			ReturnString( af->GetAnimator()->GetJointName( CLIPMODEL_ID_TO_JOINT_HANDLE( trace.c.id ) ) );
			return;
		}
	}
	ReturnString( "" );
}

/*
================
LuaThread::Event_GetTraceBody
================
*/
void idLuaThread::Event_GetTraceBody()
{
	if( trace.fraction < 1.0f && trace.c.id < 0 )
	{
		idAFEntity_Base* af = static_cast< idAFEntity_Base* >( gameLocal.entities[trace.c.entityNum] );
		if( af && af->IsType( idAFEntity_Base::Type ) && af->IsActiveAF() )
		{
			int bodyId = af->BodyForClipModelId( trace.c.id );
			idAFBody* body = af->GetAFPhysics()->GetBody( bodyId );
			if( body )
			{
				ReturnString( body->GetName() );
				return;
			}
		}
	}
	ReturnString( "" );
}

/*
================
LuaThread::Event_FadeIn
================
*/
void idLuaThread::Event_FadeIn( idVec3& color, float time )
{
	idVec4		fadeColor;
	idPlayer* player;

	player = gameLocal.GetLocalPlayer();
	if( player )
	{
		fadeColor.Set( color[0], color[1], color[2], 0.0f );
		player->playerView.Fade( fadeColor, SEC2MS( time ) );
	}
}

/*
================
LuaThread::Event_FadeOut
================
*/
void idLuaThread::Event_FadeOut( idVec3& color, float time )
{
	idVec4		fadeColor;
	idPlayer* player;

	player = gameLocal.GetLocalPlayer();
	if( player )
	{
		fadeColor.Set( color[0], color[1], color[2], 1.0f );
		player->playerView.Fade( fadeColor, SEC2MS( time ) );
	}
}

/*
================
LuaThread::Event_FadeTo
================
*/
void idLuaThread::Event_FadeTo( idVec3& color, float alpha, float time )
{
	idVec4		fadeColor;
	idPlayer* player;

	player = gameLocal.GetLocalPlayer();
	if( player )
	{
		fadeColor.Set( color[0], color[1], color[2], alpha );
		player->playerView.Fade( fadeColor, SEC2MS( time ) );
	}
}

/*
================
LuaThread::Event_SetShaderParm
================
*/
void idLuaThread::Event_SetShaderParm( int parmnum, float value )
{
	if( ( parmnum < 0 ) || ( parmnum >= MAX_GLOBAL_SHADER_PARMS ) )
	{
		Error( "shader parm index (%d) out of range", parmnum );
		return;
	}

	gameLocal.globalShaderParms[parmnum] = value;
}

/*
================
LuaThread::Event_StartMusic
================
*/
void idLuaThread::Event_StartMusic( const char* text )
{
	gameSoundWorld->PlayShaderDirectly( text );
}

/*
================
LuaThread::Event_Warning
================
*/
void idLuaThread::Event_Warning( const char* text )
{
	Warning( "%s", text );
}

/*
================
LuaThread::Event_Error
================
*/
void idLuaThread::Event_Error( const char* text )
{
	Error( "%s", text );
}

/*
================
LuaThread::Event_StrLen
================
*/
void idLuaThread::Event_StrLen( const char* string )
{
	int len;

	len = strlen( string );
	idLuaThread::ReturnInt( len );
}

/*
================
LuaThread::Event_StrLeft
================
*/
void idLuaThread::Event_StrLeft( const char* string, int num )
{
	int len;

	if( num < 0 )
	{
		idLuaThread::ReturnString( "" );
		return;
	}

	len = strlen( string );
	if( len < num )
	{
		idLuaThread::ReturnString( string );
		return;
	}

	idStr result( string, 0, num );
	idLuaThread::ReturnString( result );
}

/*
================
LuaThread::Event_StrRight
================
*/
void idLuaThread::Event_StrRight( const char* string, int num )
{
	int len;

	if( num < 0 )
	{
		idLuaThread::ReturnString( "" );
		return;
	}

	len = strlen( string );
	if( len < num )
	{
		idLuaThread::ReturnString( string );
		return;
	}

	idLuaThread::ReturnString( string + len - num );
}

/*
================
LuaThread::Event_StrSkip
================
*/
void idLuaThread::Event_StrSkip( const char* string, int num )
{
	int len;

	if( num < 0 )
	{
		idLuaThread::ReturnString( string );
		return;
	}

	len = strlen( string );
	if( len < num )
	{
		idLuaThread::ReturnString( "" );
		return;
	}

	idLuaThread::ReturnString( string + num );
}

/*
================
LuaThread::Event_StrMid
================
*/
void idLuaThread::Event_StrMid( const char* string, int start, int num )
{
	int len;

	if( num < 0 )
	{
		idLuaThread::ReturnString( "" );
		return;
	}

	if( start < 0 )
	{
		start = 0;
	}
	len = strlen( string );
	if( start > len )
	{
		start = len;
	}

	if( start + num > len )
	{
		num = len - start;
	}

	idStr result( string, start, start + num );
	idLuaThread::ReturnString( result );
}

/*
================
LuaThread::Event_StrToFloat( const char *string )
================
*/
void idLuaThread::Event_StrToFloat( const char* string )
{
	float result;

	result = atof( string );
	idLuaThread::ReturnFloat( result );
}

/*
================
LuaThread::Event_RadiusDamage
================
*/
void idLuaThread::Event_RadiusDamage( const idVec3& origin, idEntity* inflictor, idEntity* attacker, idEntity* ignore, const char* damageDefName, float dmgPower )
{
	gameLocal.RadiusDamage( origin, inflictor, attacker, ignore, ignore, damageDefName, dmgPower );
}

/*
================
LuaThread::Event_IsClient
================
*/
void idLuaThread::Event_IsClient()
{
	idLuaThread::ReturnFloat( common->IsClient() );
}

/*
================
LuaThread::Event_IsMultiplayer
================
*/
void idLuaThread::Event_IsMultiplayer()
{
	idLuaThread::ReturnFloat( common->IsMultiplayer() );
}

/*
================
LuaThread::Event_GetFrameTime
================
*/
void idLuaThread::Event_GetFrameTime()
{
	idLuaThread::ReturnFloat( MS2SEC( gameLocal.time - gameLocal.previousTime ) );
}

/*
================
LuaThread::Event_GetTicsPerSecond
================
*/
void idLuaThread::Event_GetTicsPerSecond()
{
	idLuaThread::ReturnFloat( com_engineHz_latched );
}

/*
================
LuaThread::Event_CacheSoundShader
================
*/
void idLuaThread::Event_CacheSoundShader( const char* soundName )
{
	declManager->FindSound( soundName );
}

/*
================
LuaThread::Event_DebugLine
================
*/
void idLuaThread::Event_DebugLine( const idVec3& color, const idVec3& start, const idVec3& end, const float lifetime )
{
	gameRenderWorld->DebugLine( idVec4( color.x, color.y, color.z, 0.0f ), start, end, SEC2MS( lifetime ) );
}

/*
================
LuaThread::Event_DebugArrow
================
*/
void idLuaThread::Event_DebugArrow( const idVec3& color, const idVec3& start, const idVec3& end, const int size, const float lifetime )
{
	gameRenderWorld->DebugArrow( idVec4( color.x, color.y, color.z, 0.0f ), start, end, size, SEC2MS( lifetime ) );
}

/*
================
LuaThread::Event_DebugCircle
================
*/
void idLuaThread::Event_DebugCircle( const idVec3& color, const idVec3& origin, const idVec3& dir, const float radius, const int numSteps, const float lifetime )
{
	gameRenderWorld->DebugCircle( idVec4( color.x, color.y, color.z, 0.0f ), origin, dir, radius, numSteps, SEC2MS( lifetime ) );
}

/*
================
LuaThread::Event_DebugBounds
================
*/
void idLuaThread::Event_DebugBounds( const idVec3& color, const idVec3& mins, const idVec3& maxs, const float lifetime )
{
	gameRenderWorld->DebugBounds( idVec4( color.x, color.y, color.z, 0.0f ), idBounds( mins, maxs ), vec3_origin, SEC2MS( lifetime ) );
}

/*
================
LuaThread::Event_DrawText
================
*/
void idLuaThread::Event_DrawText( const char* text, const idVec3& origin, float scale, const idVec3& color, const int align, const float lifetime )
{
	gameRenderWorld->DebugText( text, origin, scale, idVec4( color.x, color.y, color.z, 0.0f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), align, SEC2MS( lifetime ) );
}

/*
================
LuaThread::Event_InfluenceActive
================
*/
void idLuaThread::Event_InfluenceActive()
{
	idPlayer* player;

	player = gameLocal.GetLocalPlayer();
	if( player != NULL && player->GetInfluenceLevel() )
	{
		idLuaThread::ReturnInt( true );
	}
	else
	{
		idLuaThread::ReturnInt( false );
	}
}

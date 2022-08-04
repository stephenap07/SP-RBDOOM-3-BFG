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
#ifndef SCRIPT_MANAGER_H_
#define SCRIPT_MANAGER_H_

extern const idEventDef EV_LuaThread_Execute;
extern const idEventDef EV_LuaThread_SetCallback;
extern const idEventDef EV_LuaThread_TerminateThread;
extern const idEventDef EV_LuaThread_Pause;
extern const idEventDef EV_LuaThread_Wait;
extern const idEventDef EV_LuaThread_WaitFrame;
extern const idEventDef EV_LuaThread_WaitFor;
extern const idEventDef EV_LuaThread_WaitForThread;
extern const idEventDef EV_LuaThread_Print;
extern const idEventDef EV_LuaThread_PrintLn;
extern const idEventDef EV_LuaThread_Say;
extern const idEventDef EV_LuaThread_Assert;
extern const idEventDef EV_LuaThread_Trigger;
extern const idEventDef EV_LuaThread_SetCvar;
extern const idEventDef EV_LuaThread_GetCvar;
extern const idEventDef EV_LuaThread_Random;
extern const idEventDef EV_LuaThread_GetTime;
extern const idEventDef EV_LuaThread_KillThread;
extern const idEventDef EV_LuaThread_SetThreadName;
extern const idEventDef EV_LuaThread_GetEntity;
extern const idEventDef EV_LuaThread_Spawn;
extern const idEventDef EV_LuaThread_SetSpawnArg;
extern const idEventDef EV_LuaThread_SpawnString;
extern const idEventDef EV_LuaThread_SpawnFloat;
extern const idEventDef EV_LuaThread_SpawnVector;
extern const idEventDef EV_LuaThread_AngToForward;
extern const idEventDef EV_LuaThread_AngToRight;
extern const idEventDef EV_LuaThread_AngToUp;
extern const idEventDef EV_LuaThread_Sine;
extern const idEventDef EV_LuaThread_Cosine;
extern const idEventDef EV_LuaThread_Normalize;
extern const idEventDef EV_LuaThread_VecLength;
extern const idEventDef EV_LuaThread_VecDotProduct;
extern const idEventDef EV_LuaThread_VecCrossProduct;
extern const idEventDef EV_LuaThread_OnSignal;
extern const idEventDef EV_LuaThread_ClearSignal;
extern const idEventDef EV_LuaThread_SetCamera;
extern const idEventDef EV_LuaThread_FirstPerson;
extern const idEventDef EV_LuaThread_TraceFraction;
extern const idEventDef EV_LuaThread_TracePos;
extern const idEventDef EV_LuaThread_FadeIn;
extern const idEventDef EV_LuaThread_FadeOut;
extern const idEventDef EV_LuaThread_FadeTo;
extern const idEventDef EV_LuaThread_Restart;

class idLuaThread : public idClass
{
public:
	CLASS_PROTOTYPE( idLuaThread );

	idLuaThread( );
	idLuaThread( idEntity* self );

	~idLuaThread( ) override;

	lua_State* LuaState( )
	{
		return luaState;
	}

	void	Init();
	bool	LoadLuaScript( const char* luaScript, bool failIfFound = false ) const;
	void	Restart();
	void	Think();

	void	Call( const idCmdArgs& args ) const;

	void	ReturnString( const char* text ) const;
	void	ReturnFloat( float value ) const;
	void	ReturnInt( int value ) const;
	void	ReturnVector( idVec3 const& vec ) const;
	void	ReturnEntity( idEntity* ent ) const;

private:

	static trace_t				trace;

	idDict						spawnArgs;

	void						Event_Print( const char* text );
	void						Event_PrintLn( const char* text );
	void						Event_Say( const char* text );
	void						Event_Assert( float value );
	void						Event_Trigger( idEntity* ent );
	void						Event_SetCvar( const char* name, const char* value ) const;
	void						Event_GetCvar( const char* name ) const;
	void						Event_Random( float range ) const;
	void						Event_RandomInt( int range ) const;
	void						Event_GetTime();
	void						Event_GetEntity( const char* name );
	void						Event_Spawn( const char* classname );
	void						Event_CopySpawnArgs( idEntity* ent );
	void						Event_SetSpawnArg( const char* key, const char* value );
	void						Event_SpawnString( const char* key, const char* defaultvalue );
	void						Event_SpawnFloat( const char* key, float defaultvalue );
	void						Event_SpawnVector( const char* key, idVec3& defaultvalue );
	void						Event_ClearPersistantArgs();
	void 						Event_SetPersistantArg( const char* key, const char* value );
	void 						Event_GetPersistantString( const char* key );
	void 						Event_GetPersistantFloat( const char* key );
	void 						Event_GetPersistantVector( const char* key );
	void						Event_AngToForward( idAngles& ang );
	void						Event_AngToRight( idAngles& ang );
	void						Event_AngToUp( idAngles& ang );
	void						Event_GetSine( float angle );
	void						Event_GetCosine( float angle );
	void						Event_GetArcSine( float a );
	void						Event_GetArcCosine( float a );
	void						Event_GetSquareRoot( float theSquare );
	void						Event_VecNormalize( idVec3& vec );
	void						Event_VecLength( idVec3& vec );
	void						Event_VecDotProduct( idVec3& vec1, idVec3& vec2 );
	void						Event_VecCrossProduct( idVec3& vec1, idVec3& vec2 );
	void						Event_VecToAngles( idVec3& vec );
	void						Event_VecToOrthoBasisAngles( idVec3& vec );
	void						Event_RotateVector( idVec3& vec, idVec3& ang );
	void						Event_OnSignal( int signal, idEntity* ent, const char* func );
	void						Event_ClearSignalThread( int signal, idEntity* ent );
	void						Event_SetCamera( idEntity* ent );
	void						Event_FirstPerson();
	void						Event_Trace( const idVec3& start, const idVec3& end, const idVec3& mins, const idVec3& maxs, int contents_mask, idEntity* passEntity );
	void						Event_TracePoint( const idVec3& start, const idVec3& end, int contents_mask, idEntity* passEntity );
	void						Event_GetTraceFraction();
	void						Event_GetTraceEndPos();
	void						Event_GetTraceNormal();
	void						Event_GetTraceEntity();
	void						Event_GetTraceJoint();
	void						Event_GetTraceBody();
	void						Event_FadeIn( idVec3& color, float time );
	void						Event_FadeOut( idVec3& color, float time );
	void						Event_FadeTo( idVec3& color, float alpha, float time );
	void						Event_SetShaderParm( int parmnum, float value );
	void						Event_StartMusic( const char* name );
	void						Event_Warning( const char* text );
	void						Event_Error( const char* text );
	void 						Event_StrLen( const char* string );
	void 						Event_StrLeft( const char* string, int num );
	void 						Event_StrRight( const char* string, int num );
	void 						Event_StrSkip( const char* string, int num );
	void 						Event_StrMid( const char* string, int start, int num );
	void						Event_StrToFloat( const char* string );
	void						Event_RadiusDamage( const idVec3& origin, idEntity* inflictor, idEntity* attacker, idEntity* ignore, const char* damageDefName, float dmgPower );
	void						Event_IsClient();
	void 						Event_IsMultiplayer();
	void 						Event_GetFrameTime();
	void 						Event_GetTicsPerSecond();
	void						Event_CacheSoundShader( const char* soundName );
	void						Event_DebugLine( const idVec3& color, const idVec3& start, const idVec3& end, const float lifetime );
	void						Event_DebugArrow( const idVec3& color, const idVec3& start, const idVec3& end, const int size, const float lifetime );
	void						Event_DebugCircle( const idVec3& color, const idVec3& origin, const idVec3& dir, const float radius, const int numSteps, const float lifetime );
	void						Event_DebugBounds( const idVec3& color, const idVec3& mins, const idVec3& maxs, const float lifetime );
	void						Event_DrawText( const char* text, const idVec3& origin, float scale, const idVec3& color, const int align, const float lifetime );
	void						Event_InfluenceActive() const;

private:
	friend class ScriptManager;
	friend class ScopedLuaState;

	void						InitLuaState( );

	void						Error( VERIFY_FORMAT_STRING const char* fmt, ... ) const;
	void						Warning( VERIFY_FORMAT_STRING const char* fmt, ... ) const;

	lua_State* luaState;
};

class spStateScript;

class ScriptManager
{
public:

	ScriptManager();

	~ScriptManager();

	bool		Init();
	void		Shutdown();
	void		Reload();
	void		Restart();
	void		LoadScript( const char* script );
	void		AddReloadable( spStateScript* stateScript );
	void		DestroyReloadable( spStateScript* stateScript );
	void		Think();

	lua_State*	LuaState( )
	{
		return luaThread->LuaState();
	}
	idLuaThread*	GetLuaThread()
	{
		return luaThread;
	}
	static void	Call_f( const idCmdArgs& args );
	void		ReturnString( const char* text );
	void		ReturnFloat( float value );
	void		ReturnInt( int value );
	void		ReturnVector( idVec3 const& vec );
	void		ReturnEntity( idEntity* ent );

	void		Error( VERIFY_FORMAT_STRING const char* fmt, ... ) const;
	void		Warning( VERIFY_FORMAT_STRING const char* fmt, ... ) const;

private:
	friend class ScopedLuaState;

	idLuaThread*			luaThread;
	idList<spStateScript*>	reloadables;
};

/// This class updates the lua_State* to the current call so that any return
/// values are placed into the correct lua thread.
class ScopedLuaState
{
public:
	ScopedLuaState( lua_State* L );
	~ScopedLuaState( );

private:
	lua_State* originalProgramThread;
	lua_State* originalThread;
	lua_State* newThread;
};

#endif
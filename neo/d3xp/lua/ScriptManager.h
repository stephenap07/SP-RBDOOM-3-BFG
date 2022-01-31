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
#ifndef __SCRIPT_MANAGER_H__
#define __SCRIPT_MANAGER_H__

extern const idEventDef EV_LuaThread_Print;
extern const idEventDef EV_LuaThread_GetEntity;
extern const idEventDef EV_LuaThread_DrawText;

class LuaThread : public idClass
{
private:
	void	Event_Print( const char* text );
	void	Event_GetEntity( const char* name );
	void	Event_DrawText( const char* text, const idVec3& origin, float scale, const idVec3& color, const int align, const float lifetime );

public:
	CLASS_PROTOTYPE( LuaThread );

	LuaThread( );
	LuaThread( idEntity* self );

	~LuaThread( ) override;

	lua_State* LuaState( )
	{
		return luaState;
	}

	void	Init( );
	bool	LoadLuaScript( const char* luaScript, bool failIfFound = false );
	void	Restart( );

	void	Call( const char* name, int numargs );

	void	ReturnString( const char* text );
	void	ReturnFloat( float value );
	void	ReturnInt( int value );
	void	ReturnVector( idVec3 const& vec );
	void	ReturnEntity( idEntity* ent );

private:
	friend class ScriptManager;
	friend class ScopedLuaState;

	void InitLuaState( );

	lua_State* luaState;
};

class ScriptManager
{
public:

	ScriptManager();

	~ScriptManager();

	bool		Init( );
	void		Shutdown( );
	void		Reload( );
	void		Restart( );
	void		SendEvent( const char* eventName );
	void		LoadScript( const char* script );
	void		Think( int mSeconds );

	lua_State*	LuaState( )
	{
		return luaThread->LuaState();
	}
	void		Call( const char* name, int numargs );
	void		ReturnString( const char* text );
	void		ReturnFloat( float value );
	void		ReturnInt( int value );
	void		ReturnVector( idVec3 const& vec );
	void		ReturnEntity( idEntity* ent );

private:
	friend class ScopedLuaState;

	int startTime = 0;
	bool firstThink = false;
	LuaThread* luaThread;
};

/// This class updates the lua_State* to the current call so that any return
/// values are placed into the correct lua thread.
class ScopedLuaState
{
public:
	ScopedLuaState( lua_State* L );
	~ScopedLuaState( );

private:
	lua_State* originalThread;
	lua_State* newThread;
};

#endif
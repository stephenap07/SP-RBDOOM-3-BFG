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
static void TS_New(lua_State* L, idEntity* ent)
{
	// Get the register table
	lua_getglobal(L, ModuleName);
	if (!lua_istable(L, -1))
	{
		gameLocal.Warning("Failed to find the %s table", ModuleName);
		return;
	}

	lua_getfield(L, -1, RegisterName);
	if (!lua_isfunction(L, -1))
	{
		gameLocal.Warning("Failed to find function %s", RegisterName);
	}

	gameLocal.scriptManager.ReturnEntity(ent);

	if (lua_pcall(L, 1, 0, 0) != LUA_OK)
	{
		gameLocal.Warning("Something went wrong in the %s.%s: %s", ModuleName, RegisterName, lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void TS_SendEvent(lua_State* L, idEntity* ent, const char* eventName)
{
	// Get the register table
	lua_getglobal(L, ModuleName);
	if (!lua_istable(L, -1))
	{
		gameLocal.Warning("Failed to find the %s table", ModuleName);
		return;
	}

	lua_getfield(L, -1, SendEventName);
	gameLocal.scriptManager.ReturnEntity(ent);

	lua_createtable(L, 0, 1);
	lua_pushstring(L, eventName);
	lua_setfield(L, -2, "name");

	if (lua_pcall(L, 2, 0, 0) != LUA_OK)
	{
		gameLocal.Warning("Something went wrong in the %s.%s: %s", ModuleName, SendEventName, lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void TS_Think(lua_State* L, idEntity* ent)
{
	// Get the register table
	lua_getglobal(L, ModuleName);
	if (!lua_istable(L, -1))
	{
		gameLocal.Warning("Failed to find the %s table", ModuleName);
		return;
	}

	lua_getfield(L, -1, ThinkName);
	gameLocal.scriptManager.ReturnEntity(ent);

	if (lua_pcall(L, 1, 0, 0) != LUA_OK)
	{
		gameLocal.Warning("Something went wrong in the %s.%s: %s", ModuleName, ThinkName, lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

idStateScript::idStateScript(idEntity* _owner)
	: owner(_owner), scriptName()
{
}

void idStateScript::Construct()
{
	if (scriptName.IsEmpty())
	{
		return;
	}

	gameLocal.scriptManager.AddReloadable(this);

	lua_State* L = gameLocal.scriptManager.LuaState();
	TS_New(L, owner);
}

void idStateScript::Destroy()
{
	if (scriptName.IsEmpty())
	{
		return;
	}

	gameLocal.scriptManager.DestroyReloadable(this);

	lua_State* L = gameLocal.scriptManager.LuaState();

	lua_getglobal(L, owner->GetName());
	if (lua_istable(L, -1))
	{
		lua_pushnil(L);
		lua_setglobal(L, owner->GetName());
	}
	lua_pop(L, 1);
}

void idStateScript::Think()
{
	if (scriptName.IsEmpty())
	{
		return;
	}

	lua_State* L = gameLocal.scriptManager.LuaState();

	TS_Think(L, owner);
}

void idStateScript::Reload()
{
	if (scriptName.IsEmpty())
	{
		return;
	}

	Construct();
}

void idStateScript::SendEvent(int entityNumber, const char* eventName)
{
	if (scriptName.IsEmpty())
	{
		return;
	}

	lua_State* L = gameLocal.scriptManager.LuaState();

	TS_SendEvent(L, owner, eventName);
}
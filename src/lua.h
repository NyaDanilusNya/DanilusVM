#ifndef DANILUS_LUA_H
#define DANILUS_LUA_H

#include <lua5.2/lua.h>

void lua_DeInitLua();
int lua_InitLua(/*lua_State* NL*/);

void lua_Start();

#endif

#ifndef CMAKE_CASSANDRA_LUA_LIBRARY_H
#define CMAKE_CASSANDRA_LUA_LIBRARY_H

#include <luajit-2.1/lua.h>

int luaopen_luacassandra(lua_State *L);
static int connect(lua_State *L);

#endif
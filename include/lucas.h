#define LUCAS_VERSION_MAJOR 0
#define LUCAS_VERSION_MINOR 0
#define LUCAS_VERSION_PATCH 1

#ifndef CMAKE_CASSANDRA_LUA_LIBRARY_H
#define CMAKE_CASSANDRA_LUA_LIBRARY_H

#include <luajit-2.1/lua.h>

int luaopen_lucas(lua_State *L);

#endif

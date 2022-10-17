#ifndef CMAKE_CASSANDRA_LUA_LIBRARY_H
#define CMAKE_CASSANDRA_LUA_LIBRARY_H
extern "C" {
    #include <luajit-2.1/lua.h>

    int luaopen_helper(lua_State *l);
}
#endif

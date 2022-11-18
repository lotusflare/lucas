#include "cassandra.h"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>

static int convert(lua_State *L)
{
    const int ARG_PARAM = 1;
    int lt = lua_type(L, ARG_PARAM);
    lua_newtable(L);
    int table = lua_gettop(L);

    if (lt == LUA_TSTRING || lt == LUA_TBOOLEAN || lt == LUA_TNUMBER)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_VARCHAR);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, ARG_PARAM);
        lua_rawseti(L, table, 2);
    }
    else if (lt == LUA_TTABLE)
    {
        lua_getfield(L, ARG_PARAM, "__cql_type");
        if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
        {
            lua_pushinteger(L, CASS_VALUE_TYPE_NULL);
            lua_rawseti(L, table, 1);
        }
        else
        {
            lua_getfield(L, ARG_PARAM, "__cql_type");
            lua_rawseti(L, table, 1);
            lua_getfield(L, ARG_PARAM, "val");
            lua_rawseti(L, table, 2);
        }
        lua_pop(L, 1);
    }
    return 1;
}

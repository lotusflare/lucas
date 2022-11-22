#include "cassandra.h"
#include "errors.c"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>

static int convert(lua_State *L)
{
    const int ARG_PARAM = 1;
    int lt = lua_type(L, ARG_PARAM);
    lua_newtable(L);
    int table = lua_gettop(L);

    if (lt == LUA_TSTRING)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_VARCHAR);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, ARG_PARAM);
        lua_rawseti(L, table, 2);
    }
    else if (lt == LUA_TBOOLEAN)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_BOOLEAN);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, ARG_PARAM);
        lua_rawseti(L, table, 2);
    }
    else if (lt == LUA_TNUMBER)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_INT);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, ARG_PARAM);
        lua_rawseti(L, table, 2);
    }
    else if (lt == LUA_TTABLE)
    {
        lua_getfield(L, ARG_PARAM, "__cql_type");
        if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
        {
            lua_pushinteger(L, CASS_VALUE_TYPE_UNSET);
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
    else
    {
        lucas_error_to_lua(L, lucas_new_errorf("could not convert type %d", lt));
    }
    return 1;
}

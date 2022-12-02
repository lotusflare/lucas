#include "cassandra.h"
#include "errors.c"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>

void cast(lua_State *L, int value_index)
{
    lua_newtable(L);
    int table = lua_gettop(L);
    int lt = lua_type(L, value_index);

    if (lt == LUA_TSTRING)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_TEXT);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
    }
    else if (lt == LUA_TBOOLEAN)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_BOOLEAN);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
    }
    else if (lt == LUA_TNUMBER)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_INT);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
    }
}

LucasError *iterate_list(lua_State *L, int index, int table)
{
    lua_newtable(L);
    const int list_table = lua_gettop(L);
    lua_pushnil(L);
    int item_count = 0;
    bool is_map = false;

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int key_type = lua_type(L, key_index);
        const int value_index = lua_gettop(L);

        if (key_type == LUA_TSTRING)
        {
            cast(L, key_index);
            is_map = true;
        }
        else if (key_type == LUA_TNUMBER)
        {
            lua_pushinteger(L, ++item_count);
        }
        cast(L, value_index);
        lua_settable(L, list_table);
    }

    if (is_map)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_MAP);
        lua_rawseti(L, table, 1);
        lua_rawseti(L, table, 2);
    }
    else if (item_count > 0)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_LIST);
        lua_rawseti(L, table, 1);
        lua_rawseti(L, table, 2);
    }
    else
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_NULL);
        lua_rawseti(L, table, 1);
        lua_pop(L, 1);
    }

    return NULL;
}

static int convert(lua_State *L)
{
    const int ARG_PARAM = 1;
    int lt = lua_type(L, ARG_PARAM);

    if (lt == LUA_TSTRING || lt == LUA_TBOOLEAN || lt == LUA_TNUMBER)
    {
        cast(L, ARG_PARAM);
        return 1;
    }
    else if (lt == LUA_TTABLE)
    {
        lua_newtable(L);
        const int new_table = lua_gettop(L);
        lua_getfield(L, ARG_PARAM, "__cql_type");

        if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
        {
            CassValueType type = lua_tointeger(L, lua_gettop(L));
            if (type != CASS_VALUE_TYPE_LIST && type != CASS_VALUE_TYPE_MAP && type != CASS_VALUE_TYPE_SET)
            {
                lua_rawseti(L, new_table, 1);
                lua_getfield(L, ARG_PARAM, "val");
                lua_rawseti(L, new_table, 2);
                return 1;
            }
            else
            {
                lua_pop(L, 1);
                lua_getfield(L, ARG_PARAM, "val");
                iterate_list(L, lua_gettop(L), new_table);
                lua_pop(L, 1);
                return 1;
            }
        }
        else
        {
            lua_pop(L, 1);
            iterate_list(L, ARG_PARAM, new_table);
            return 1;
        }
    }
    else
    {
        lucas_error_to_lua(L, lucas_new_errorf("could not convert type %d", lt));
    }
    return 0;
}

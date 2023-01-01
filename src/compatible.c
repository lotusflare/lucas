#include "cassandra.h"
#include "compatibility.h"
#include "errors.c"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <stdio.h>

LucasError *cast(lua_State *L, int value_index)
{
    lua_newtable(L);
    int table = lua_gettop(L);
    int type = lua_type(L, value_index);

    if (type == LUA_TSTRING)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_TEXT);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
    }
    else if (type == LUA_TBOOLEAN)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_BOOLEAN);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
    }
    else if (type == LUA_TNUMBER)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_INT);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
    }
    else if (type == LUA_TTABLE)
    {
        return convert_table(L, value_index, table);
    }

    return lucas_new_errorf("invalid type");
}

LucasError *convert_list(lua_State *L, int index, int table, CassValueType *type)
{
    lua_newtable(L);
    const int collection_table = lua_gettop(L);
    lua_pushnil(L);

    int item_count = 1;
    bool is_map = false;
    LucasError *rc = NULL;

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int key_type = lua_type(L, key_index);
        const int value_index = lua_gettop(L);

        if (key_type == LUA_TSTRING || key_type == LUA_TTABLE)
        {
            is_map = true;
            rc = cast(L, key_index);
            if (rc)
            {
                return rc;
            }
            rc = cast(L, value_index);
            if (rc)
            {
                return rc;
            }
        }
        else if (key_type == LUA_TNUMBER)
        {
            lua_pushinteger(L, item_count);
            rc = cast(L, value_index);
            if (rc)
            {
                return rc;
            }
        }
        else
        {
            return lucas_new_errorf("invalid key type");
        }
        lua_settable(L, collection_table);
        item_count++;
    }

    // determine type
    if (!type)
    {
        CassValueType cvt_fallback = is_map ? CASS_VALUE_TYPE_MAP : CASS_VALUE_TYPE_LIST;
        type = &cvt_fallback;
    }

    lua_pushinteger(L, *type);
    lua_rawseti(L, table, 1);
    lua_rawseti(L, table, 2);

    return NULL;
}

bool table_empty(lua_State *L, int index)
{
    int top = lua_gettop(L);
    lua_pushnil(L);
    bool is_empty = lua_next(L, index) == 0;
    lua_settop(L, top);
    return is_empty;
}

bool get_cql_type(lua_State *L, int index, CassValueType *cvt)
{
    int top = lua_gettop(L);
    lua_getfield(L, index, "__cql_type");
    bool has_type = lua_type(L, lua_gettop(L)) == LUA_TNIL;
    *cvt = lua_tointeger(L, lua_gettop(L));
    lua_settop(L, top);
    return has_type;
}

LucasError *convert_table(lua_State *L, int index, int return_table)
{
    bool is_empty = table_empty(L, index);
    if (is_empty)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_NULL);
        lua_rawseti(L, return_table, 1);
        return NULL;
    }

    CassValueType cvt;
    bool has_type = get_cql_type(L, index, &cvt);
    if (!has_type)
    {
        return convert_list(L, index, return_table, NULL);
    }
    else if (cvt == CASS_VALUE_TYPE_LIST || cvt == CASS_VALUE_TYPE_MAP || cvt == CASS_VALUE_TYPE_SET)
    {
        lua_getfield(L, index, "val");
        int val_index = lua_gettop(L);
        LucasError *rc = convert_list(L, val_index, return_table, &cvt);
        lua_pop(L, 1);
        return rc;
    }

    lua_getfield(L, index, "__cql_type");
    lua_rawseti(L, return_table, 1);
    lua_getfield(L, index, "val");
    lua_rawseti(L, return_table, 2);
    return NULL;
}

static int convert(lua_State *L)
{
    const int ARG_PARAM = 1;
    int type = lua_type(L, ARG_PARAM);
    LucasError *rc = cast(L, ARG_PARAM);
    if (rc)
    {
        lucas_error_to_lua(L, rc);
    }
    return 1;
}

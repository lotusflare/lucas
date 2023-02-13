#include "compatible.h"
#include "cassandra.h"
#include "errors.c"
#include "logs.c"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <stdio.h>

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
    bool has_type = !lua_isnil(L, lua_gettop(L));
    *cvt = lua_tointeger(L, lua_gettop(L));
    lua_settop(L, top);
    return has_type;
}

LucasError *handle(lua_State *L, int value_index)
{
    lua_newtable(L);
    const int table = lua_gettop(L);
    const int type = lua_type(L, value_index);

    if (type == LUA_TSTRING)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_TEXT);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
        return NULL;
    }

    if (type == LUA_TBOOLEAN)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_BOOLEAN);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
        return NULL;
    }

    if (type == LUA_TNUMBER)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_INT);
        lua_rawseti(L, table, 1);
        lua_pushvalue(L, value_index);
        lua_rawseti(L, table, 2);
        return NULL;
    }

    if (type == LUA_TTABLE)
    {
        return handle_table(L, value_index, table);
    }

    return lucas_new_errorf("invalid type");
}

LucasError *handle_collection(lua_State *L, int index, int table, CassValueType *cvt_override)
{
    lua_newtable(L);
    const int collection_table = lua_gettop(L);
    lua_pushnil(L);

    int item_count = 1;
    bool is_map = false;
    LucasError *rc = NULL;
    CassValueType cvt = *cvt_override;

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_settop(L, last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int key_type = lua_type(L, key_index);
        const int value_index = lua_gettop(L);

        if (key_type == LUA_TSTRING || key_type == LUA_TTABLE)
        {
            is_map = true;
            rc = handle(L, key_index);
            if (rc)
            {
                return rc;
            }
            rc = handle(L, value_index);
            if (rc)
            {
                return rc;
            }
        }
        else if (key_type == LUA_TNUMBER)
        {
            lua_pushinteger(L, item_count);
            rc = handle(L, value_index);
            if (rc)
            {
                return rc;
            }
        }
        else
        {
            return lucas_new_errorf("invalid key type: %d", key_type);
        }
        lua_settable(L, collection_table);
        item_count++;
    }

    if (!cvt)
    {
        cvt = is_map ? CASS_VALUE_TYPE_MAP : CASS_VALUE_TYPE_LIST;
        lucas_log(LOG_DEBUG, "no collection type specified, defaulting to %d", cvt);
    }

    lua_pushinteger(L, cvt);
    lua_rawseti(L, table, 1);
    lua_rawseti(L, table, 2);
    return NULL;
}

LucasError *handle_table(lua_State *L, int index, int return_table)
{
    bool is_empty = table_empty(L, index);
    if (is_empty)
    {
        lucas_log(LOG_DEBUG, "collection is empty, returning");
        lua_pushinteger(L, CASS_VALUE_TYPE_NULL);
        lua_rawseti(L, return_table, 1);
        return NULL;
    }

    CassValueType cvt;
    bool has_type = get_cql_type(L, index, &cvt);
    if (!has_type)
    {
        return handle_collection(L, index, return_table, NULL);
    }

    if (cvt == CASS_VALUE_TYPE_LIST || cvt == CASS_VALUE_TYPE_MAP || cvt == CASS_VALUE_TYPE_SET)
    {
        lua_getfield(L, index, "val");
        handle_collection(L, lua_gettop(L), return_table, &cvt);
        lua_pop(L, 1);
        return NULL;
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
    LucasError *rc = handle(L, ARG_PARAM);
    if (rc)
    {
        lucas_error_to_lua(L, rc);
    }
    return 1;
}

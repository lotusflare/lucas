#include "compatibility.h"
#include "cassandra.h"
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
    LucasError *rc = NULL;

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
        rc = convert_table(L, value_index, table);
    }
    else
    {
        return lucas_new_errorf("invalid type");
    }

    return NULL;
}

LucasError *convert_list(lua_State *L, int index, int table, CassValueType *type)
{
    lua_newtable(L);
    const int collection_table = lua_gettop(L);
    lua_pushnil(L);

    int item_count = 0;
    LucasError *rc = NULL;

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int key_type = lua_type(L, key_index);
        const int value_index = lua_gettop(L);
        lua_pushinteger(L, ++item_count);

        if (key_type == LUA_TSTRING || key_type == LUA_TTABLE)
        {
            lua_newtable(L);
            const int tuple_table = lua_gettop(L);
            rc = cast(L, key_index);
            if (rc)
            {
                return rc;
            }
            lua_rawseti(L, tuple_table, 1);
            rc = cast(L, value_index);
            if (rc)
            {
                return rc;
            }
            lua_rawseti(L, tuple_table, 2);
        }
        else if (key_type == LUA_TNUMBER)
        {
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
    }

    if (item_count > 0)
    {
        lua_pushinteger(L, *type);
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

LucasError *convert_table(lua_State *L, int index, int return_table)
{
    printf("convert_table\n");

    LucasError *rc = NULL;

    // check if table is empty
    bool is_empty = true;
    lua_pushnil(L);
    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        is_empty = false;
        break;
    }
    if (is_empty)
    {
        lua_pushinteger(L, CASS_VALUE_TYPE_NULL);
        lua_rawseti(L, return_table, 1);
        printf("is_empty top=%d\n", lua_gettop(L));
        return NULL;
    }
    lua_pop(L, 2);
    printf("not empty\n");

    // check if is CQL type
    lua_getfield(L, index, "__cql_type");
    CassValueType cvt = lua_tointeger(L, lua_gettop(L));
    int lt = lua_type(L, lua_gettop(L));
    lua_pop(L, 1);

    if (cvt == CASS_VALUE_TYPE_LIST || cvt == CASS_VALUE_TYPE_MAP || cvt == CASS_VALUE_TYPE_SET)
    {
        lua_getfield(L, index, "__cql_type");
        lua_rawseti(L, return_table, 1);
        lua_getfield(L, index, "val");
        lua_newtable(L);
        convert_table(L, lua_gettop(L) - 1, lua_gettop(L));
        lua_rawseti(L, return_table, 2);
        lua_pop(L, 1);
        return NULL;
    }
    else if (lt != LUA_TNIL)
    {
        lua_getfield(L, index, "__cql_type");
        lua_rawseti(L, return_table, 1);
        lua_getfield(L, index, "val");
        lua_rawseti(L, return_table, 2);
        return NULL;
    }
    printf("not __cql_type\n");

    // convert each item

    // lua_pushinteger(L, 1);
    // lua_pushstring(L, "hello");
    // lua_settable(L, return_table);
    // return NULL;

    lua_pushnil(L);
    int i = 1;
    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int val_index = lua_gettop(L);
        // const int lt = lua_type(L, key_index);
        // int num = lua_tonumber(L, val_index);
        // printf("num=%d", num);

        rc = cast(L, val_index);
        lua_rawseti(L, return_table, i++);
    }
    // lua_pop(L, 2);
    // printf("return_table=%d\n", return_table);

    // printf("lt=%d\n", lt);
    // lua_newtable(L);

    // if (lt == LUA_TNIL)
    // {
    //     printf("NIL\n");
    //     lua_pop(L, 1);
    //     rc = convert_list(L, index, return_table, NULL);
    // }

    // if (cvt == CASS_VALUE_TYPE_LIST || cvt == CASS_VALUE_TYPE_MAP || cvt == CASS_VALUE_TYPE_SET)
    // {
    //     lua_pop(L, 1);
    //     lua_getfield(L, index, "val");
    //     rc = convert_list(L, lua_gettop(L), return_table, &cvt);
    //     lua_pop(L, 1);
    // }

    return rc;
}

static int convert(lua_State *L)
{
    const int ARG_PARAM = 1;
    int type = lua_type(L, ARG_PARAM);
    LucasError *rc = cast(L, ARG_PARAM);
    printf("top=%d\n", lua_gettop(L));

done:
    if (rc)
    {
        lucas_error_to_lua(L, rc);
    }
    return 1;
}

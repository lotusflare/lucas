#pragma once

#include "errors.c"
#include "luajit-2.1/lua.h"
#include <cassandra.h>

const int CASS_VALUE_TYPE_NULL = -1;
const int CASS_VALUE_TYPE_UNSET = -2;

int string_helper(lua_State *L, CassValueType cass_type)
{
    const char *value = lua_tostring(L, 1);
    lua_newtable(L);
    int table = lua_gettop(L);

    lua_pushinteger(L, cass_type);
    lua_rawseti(L, table, 1);

    lua_pushstring(L, value);
    lua_rawseti(L, table, 2);

    return 1;
}

int integer_helper(lua_State *L, CassValueType cass_type)
{
    lua_Integer value = lua_tointeger(L, 1);
    lua_newtable(L);
    int table = lua_gettop(L);

    lua_pushinteger(L, cass_type);
    lua_rawseti(L, table, 1);

    lua_pushinteger(L, value);
    lua_rawseti(L, table, 2);

    return 1;
}

int number_helper(lua_State *L, CassValueType cass_type)
{
    lua_Number value = lua_tonumber(L, 1);
    lua_newtable(L);
    int table = lua_gettop(L);

    lua_pushinteger(L, cass_type);
    lua_rawseti(L, table, 1);

    lua_pushnumber(L, value);
    lua_rawseti(L, table, 2);

    return 1;
}

static int type_boolean(lua_State *L)
{
    lua_newtable(L);
    int table = lua_gettop(L);

    lua_pushinteger(L, CASS_VALUE_TYPE_BOOLEAN);
    lua_rawseti(L, table, 1);

    lua_pushboolean(L, lua_toboolean(L, 1));
    lua_rawseti(L, table, 2);

    return 1;
}

static int type_null(lua_State *L)
{
    lua_newtable(L);
    int table = lua_gettop(L);

    lua_pushinteger(L, CASS_VALUE_TYPE_NULL);
    lua_rawseti(L, table, 1);

    return 1;
}

static int type_unset(lua_State *L)
{
    lua_newtable(L);
    int table = lua_gettop(L);

    lua_pushinteger(L, CASS_VALUE_TYPE_UNSET);
    lua_rawseti(L, table, 1);

    return 1;
}

static int type_ascii(lua_State *L)
{
    return string_helper(L, CASS_VALUE_TYPE_ASCII);
}

static int type_bigint(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_BIGINT);
}

static int type_blob(lua_State *L)
{
    return string_helper(L, CASS_VALUE_TYPE_BLOB);
}

static int type_counter(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_COUNTER);
}

static int type_decimal(lua_State *L)
{
    return number_helper(L, CASS_VALUE_TYPE_DECIMAL);
}

static int type_double(lua_State *L)
{
    return number_helper(L, CASS_VALUE_TYPE_DOUBLE);
}

static int type_float(lua_State *L)
{
    return number_helper(L, CASS_VALUE_TYPE_FLOAT);
}

static int type_int(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_INT);
}

static int type_text(lua_State *L)
{
    return string_helper(L, CASS_VALUE_TYPE_TEXT);
}

static int type_timestamp(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_TIMESTAMP);
}

static int type_uuid(lua_State *L)
{
    return string_helper(L, CASS_VALUE_TYPE_UUID);
}

static int type_varchar(lua_State *L)
{
    return string_helper(L, CASS_VALUE_TYPE_VARCHAR);
}

static int type_varint(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_VARINT);
}

static int type_timeuuid(lua_State *L)
{
    return string_helper(L, CASS_VALUE_TYPE_TIMEUUID);
}

static int type_inet(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_INET);
}

static int type_date(lua_State *L)
{
    return string_helper(L, CASS_VALUE_TYPE_DATE);
}

static int type_time(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_TIME);
}

static int type_smallint(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_SMALL_INT);
}

static int type_tinyint(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_TINY_INT);
}

static int type_duration(lua_State *L)
{
    return integer_helper(L, CASS_VALUE_TYPE_DURATION);
}

static int type_list(lua_State *L)
{
    errorf_to_lua(L, "list type not supported");
    return 0;
}

static int type_map(lua_State *L)
{
    errorf_to_lua(L, "map type not supported");
    return 0;
}

static int type_set(lua_State *L)
{
    errorf_to_lua(L, "set type not supported");
    return 0;
}

static int type_udt(lua_State *L)
{
    errorf_to_lua(L, "udt type not supported");
    return 0;
}

static int type_tuple(lua_State *L)
{
    errorf_to_lua(L, "tuple type not supported");
    return 0;
}

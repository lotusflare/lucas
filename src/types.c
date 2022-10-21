#include "luajit-2.1/lua.h"
#include <cassandra.h>

static int type_ascii(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_ASCII);
    return 1;
}

static int type_bigint(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_BIGINT);
    return 1;
}

static int type_blob(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_BLOB);
    return 1;
}

static int type_boolean(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_BOOLEAN);
    return 1;
}

static int type_counter(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_COUNTER);
    return 1;
}

static int type_decimal(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_DECIMAL);
    return 1;
}

static int type_double(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_DOUBLE);
    return 1;
}

static int type_float(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_FLOAT);
    return 1;
}

static int type_int(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_INT);
    return 1;
}

static int type_text(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_TEXT);
    return 1;
}

static int type_timestamp(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_TIMESTAMP);
    return 1;
}

static int type_uuid(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_UUID);
    return 1;
}

static int type_varchar(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_VARCHAR);
    return 1;
}

static int type_varint(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_VARINT);
    return 1;
}

static int type_timeuuid(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_TIMEUUID);
    return 1;
}

static int type_inet(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_INET);
    return 1;
}

static int type_date(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_DATE);
    return 1;
}

static int type_time(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_TIME);
    return 1;
}

static int type_smallint(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_SMALL_INT);
    return 1;
}

static int type_tinyint(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_TINY_INT);
    return 1;
}

static int type_duration(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_DURATION);
    return 1;
}

static int type_list(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_LIST);
    return 1;
}

static int type_map(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_MAP);
    return 1;
}

static int type_set(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_SET);
    return 1;
}

static int type_udt(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_UDT);
    return 1;
}

static int type_tuple(lua_State *L)
{
    lua_pushinteger(L, CASS_VALUE_TYPE_TUPLE);
    return 1;
}

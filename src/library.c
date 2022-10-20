#include "library.h"
#include "cassandra.h"
#include "errors.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "types.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CassSession *session;
CassCluster *cluster;

const int QUERY_POSITION = 1;
const int PARAMETERS_POSITION = 2;

const int TYPE_POSITION = 1;
const int TYPE_OFFSET = -2;
const int VALUE_POSITION = 2;
const int VALUE_OFFSET = -1;

const int CONTACT_POINTS_POSITION = 1;

static int connect(lua_State *L)
{
    luaL_checkstring(L, CONTACT_POINTS_POSITION);
    const char *contact_points = lua_tostring(L, CONTACT_POINTS_POSITION);
    CassError err;
    session = cass_session_new();
    cluster = cass_cluster_new();
    err = cass_cluster_set_contact_points(cluster, contact_points);
    if (err != CASS_OK)
    {
        error_cass_to_lua(L, err, "could not set contact points");
    }
    err = cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V4);
    if (err != CASS_OK)
    {
        error_cass_to_lua(L, err, "could not set protocol version");
    }
    CassFuture *future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    cass_future_free(future);
    if (err != CASS_OK)
    {
        error_cass_to_lua(L, err, "could not connect");
    }
    return 0;
}

void bind_positional_parameter(lua_State *L, int i, CassStatement *statement, CassValueType type, int index)
{
    CassError err;

    if (type == CASS_VALUE_TYPE_UUID || type == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid uuid;
        err = cass_uuid_from_string(lua_tostring(L, index), &uuid);
        cass_statement_bind_uuid(statement, i, uuid);
    }
    else if (type == CASS_VALUE_TYPE_TINY_INT)
    {
        err = cass_statement_bind_int8(statement, i, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_SMALL_INT)
    {
        err = cass_statement_bind_int16(statement, i, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_INT)
    {
        err = cass_statement_bind_int32(statement, i, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_BIGINT)
    {
        err = cass_statement_bind_int64(statement, i, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_FLOAT)
    {
        err = cass_statement_bind_float(statement, i, lua_tonumber(L, index));
    }
    else if (type == CASS_VALUE_TYPE_DOUBLE)
    {
        err = cass_statement_bind_double(statement, i, lua_tonumber(L, index));
    }
    else if (type == CASS_VALUE_TYPE_ASCII || type == CASS_VALUE_TYPE_TEXT || type == CASS_VALUE_TYPE_VARCHAR)
    {
        err = cass_statement_bind_string(statement, i, lua_tostring(L, index));
    }
    else
    {
        error_to_lua(L, "invalid type: %d", type);
    }

    if (err != CASS_OK)
    {
        error_cass_to_lua(L, err, "error binding positional parameter: %d", type);
    }
}

void bind_named_parameter(lua_State *L, const char *name, CassStatement *statement, CassValueType type, int index)
{
    CassError err;

    if (type == CASS_VALUE_TYPE_UUID || type == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid uuid;
        err = cass_uuid_from_string(lua_tostring(L, index), &uuid);
        cass_statement_bind_uuid_by_name(statement, name, uuid);
    }
    else if (type == CASS_VALUE_TYPE_TINY_INT)
    {
        err = cass_statement_bind_int8_by_name(statement, name, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_SMALL_INT)
    {
        err = cass_statement_bind_int16_by_name(statement, name, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_INT)
    {
        err = cass_statement_bind_int32_by_name(statement, name, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_BIGINT)
    {
        err = cass_statement_bind_int64_by_name(statement, name, lua_tointeger(L, index));
    }
    else if (type == CASS_VALUE_TYPE_FLOAT)
    {
        err = cass_statement_bind_float_by_name(statement, name, lua_tonumber(L, index));
    }
    else if (type == CASS_VALUE_TYPE_DOUBLE)
    {
        err = cass_statement_bind_double_by_name(statement, name, lua_tonumber(L, index));
    }
    else if (type == CASS_VALUE_TYPE_ASCII || type == CASS_VALUE_TYPE_TEXT || type == CASS_VALUE_TYPE_VARCHAR)
    {
        err = cass_statement_bind_string_by_name(statement, name, lua_tostring(L, index));
    }
    else
    {
        error_to_lua(L, "invalid type: %d", type);
    }

    if (err != CASS_OK)
    {
        error_cass_to_lua(L, err, "error binding positional parameter: %d", type);
    }
}

CassStatement *create_statement_old(lua_State *L)
{
    const size_t parameter_count = lua_objlen(L, PARAMETERS_POSITION);
    const char *query = lua_tostring(L, QUERY_POSITION);
    CassStatement *statement = cass_statement_new(query, parameter_count);

    for (size_t i = 0; i < parameter_count; i++)
    {
        lua_rawgeti(L, PARAMETERS_POSITION, i + 1);
        int current = lua_gettop(L);
        lua_rawgeti(L, current, TYPE_POSITION);
        lua_rawgeti(L, current, VALUE_POSITION);
        luaL_checkinteger(L, TYPE_OFFSET);
        const CassValueType type = lua_tointeger(L, TYPE_OFFSET);
        bind_positional_parameter(L, i, statement, type, VALUE_OFFSET);
        lua_pop(L, 2);
    }

    return statement;
}

void create_statement(lua_State *L, CassStatement *statement)
{
    for (lua_pushnil(L); lua_next(L, PARAMETERS_POSITION) != 0;)
    {
        const int table_key_index = lua_gettop(L) - 1;
        const int table_value_index = lua_gettop(L);
        const int table_key_type = lua_type(L, table_key_index);
        lua_pushvalue(L, table_key_index); // copy the key
        const int table_key_copy_index = lua_gettop(L);

        lua_rawgeti(L, table_value_index, TYPE_POSITION);
        lua_rawgeti(L, table_value_index, VALUE_POSITION);
        const int type_index = lua_gettop(L) - 1;
        const int value_index = lua_gettop(L);
        const CassValueType type = lua_tointeger(L, type_index);
        const char *value = lua_tostring(L, value_index);

        if (table_key_type == LUA_TSTRING)
        {
            const char *name = lua_tostring(L, table_key_copy_index);
            bind_named_parameter(L, name, statement, type, value_index);
        }
        else if (table_key_type == LUA_TNUMBER)
        {
            const int index = lua_tointeger(L, table_key_copy_index) - 1; // lua base 1 indexing
            bind_positional_parameter(L, index, statement, type, value_index);
        }
        lua_pop(L, 4);
    }
}

void iterate_result(lua_State *L, const CassResult *result)
{
    CassIterator *iterator = cass_iterator_from_result(result);
    size_t col_count = cass_result_column_count(result);

    lua_newtable(L);
    const int main_table = lua_gettop(L);

    for (int table_index = 1; cass_iterator_next(iterator); table_index++)
    {
        lua_pushinteger(L, table_index);
        lua_newtable(L);
        int sub_table = lua_gettop(L);
        const CassRow *row = cass_iterator_get_row(iterator);

        for (size_t c = 0; c < col_count; c++)
        {
            const char *col_name;
            size_t col_name_len;
            cass_result_column_name(result, c, &col_name, &col_name_len);
            char terminated[col_name_len + 1];
            strncpy(terminated, col_name, col_name_len + 1);

            const CassValue *cass_value = cass_row_get_column(row, c);
            const CassDataType *dt = cass_result_column_data_type(result, c);
            const CassValueType vt = cass_data_type_type(dt);

            if (vt == CASS_VALUE_TYPE_ASCII || vt == CASS_VALUE_TYPE_TEXT || vt == CASS_VALUE_TYPE_VARCHAR)
            {
                const char *value;
                size_t length;
                cass_value_get_string(cass_value, &value, &length);
                lua_pushstring(L, terminated);
                lua_pushstring(L, value);
            }
            else if (vt == CASS_VALUE_TYPE_UUID || vt == CASS_VALUE_TYPE_TIMEUUID)
            {
                char uuid_str_val[CASS_UUID_STRING_LENGTH];
                CassUuid uuid_val;
                cass_value_get_uuid(cass_value, &uuid_val);
                cass_uuid_string(uuid_val, uuid_str_val);
                lua_pushstring(L, terminated);
                lua_pushstring(L, uuid_str_val);
            }
            else if (vt == CASS_VALUE_TYPE_INT)
            {
                cass_int32_t value;
                cass_value_get_int32(cass_value, &value);
                lua_pushstring(L, terminated);
                lua_pushinteger(L, value);
            }
            else if (vt == CASS_VALUE_TYPE_BOOLEAN)
            {
                cass_bool_t value;
                cass_value_get_bool(cass_value, &value);
                lua_pushstring(L, terminated);
                lua_pushboolean(L, value);
            }
            else if (vt == CASS_VALUE_TYPE_DOUBLE)
            {
                cass_double_t value;
                cass_value_get_double(cass_value, &value);
                lua_pushstring(L, terminated);
                lua_pushnumber(L, value);
            }
            else
            {
                lua_pushstring(L, terminated);
                lua_pushnil(L);
            }
            lua_settable(L, sub_table);
        }
        lua_settable(L, main_table);
    }
    // cass_result_free(result);
    // cass_iterator_free(iterator);
}

static int query(lua_State *L)
{
    luaL_checktype(L, QUERY_POSITION, LUA_TSTRING);
    luaL_checktype(L, PARAMETERS_POSITION, LUA_TTABLE);

    const size_t parameter_count = lua_objlen(L, PARAMETERS_POSITION);
    const char *query = lua_tostring(L, QUERY_POSITION);
    printf("%zu: %s", parameter_count, query);
    CassStatement *statement = cass_statement_new(query, parameter_count);
    create_statement(L, statement);

    CassFuture *future = cass_session_execute(session, statement);
    cass_future_wait(future);
    CassError err = cass_future_error_code(future);
    printf("%d", err);
    const CassResult *result = cass_future_get_result(future);
    iterate_result(L, result);

    // cass_future_free(future);
    // cass_statement_free(statement);

    cass_session_free(session);

    return 1;
}

int luaopen_luacassandra(lua_State *L)
{
    cass_log_set_level(CASS_LOG_DISABLED);
    luaL_Reg reg[] = {
        {"connect", connect},
        {"query", query},

        {"ascii", type_ascii},
        {"bigint", type_bigint},
        {"blob", type_blob},
        {"boolean", type_boolean},
        {"counter", type_counter},
        {"decimal", type_decimal},
        {"double", type_double},
        {"float", type_float},
        {"int", type_int},
        {"text", type_text},
        {"timestamp", type_timestamp},
        {"uuid", type_uuid},
        {"varchar", type_varchar},
        {"varint", type_varint},
        {"timeuuid", type_timeuuid},
        {"inet", type_inet},
        {"date", type_date},
        {"time", type_time},
        {"smallint", type_smallint},
        {"tinyint", type_tinyint},
        {"duration", type_duration},
        {"list", type_list},
        {"map", type_map},
        {"set", type_set},
        {"udt", type_udt},
        {"tuple", type_tuple},

        {NULL, NULL},
    };
    luaL_openlib(L, "luacassandra", reg, 0);
    return 1;
}

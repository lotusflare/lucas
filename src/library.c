#include "library.h"
#include "cassandra.h"
#include "errors.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CassSession *session;
CassCluster *cluster;

const int QUERY_POSITION = 1;
const int PARAMETERS_POSITION = 2;
const int TYPE_POSITION = 1;
const int VALUE_POSITION = 2;

static int connect(lua_State *L)
{
    luaL_checkstring(L, 1);
    const char *contact_points = lua_tostring(L, 1);
    CassError err;
    session = cass_session_new();
    cluster = cass_cluster_new();
    err = cass_cluster_set_contact_points(cluster, contact_points);
    if (err != CASS_OK)
    {
        error_cass_to_lua(L, err, "could not set contact points");
    }
    CassFuture *future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    cass_future_free(future);
    if (err != CASS_OK)
    {
        error_cass_to_lua(L, err, "could not connect");
    }
    lua_newtable(L);
    return 0;
}

CassStatement *create_statement(lua_State *L)
{
    const size_t parameter_count = lua_objlen(L, PARAMETERS_POSITION);
    const char *query = lua_tostring(L, QUERY_POSITION);
    CassStatement *statement = cass_statement_new(query, parameter_count);
    CassError err;

    for (size_t i = 0; i < parameter_count; i++)
    {
        lua_rawgeti(L, PARAMETERS_POSITION, i + 1);
        int current = lua_gettop(L);
        lua_rawgeti(L, current, TYPE_POSITION);
        lua_rawgeti(L, current, VALUE_POSITION);
        const CassValueType type = lua_tointeger(L, -2);

        if (type == CASS_VALUE_TYPE_UUID | type == CASS_VALUE_TYPE_TIMEUUID)
        {
            CassUuid uuid;
            err = cass_uuid_from_string(lua_tostring(L, -1), &uuid);
            cass_statement_bind_uuid(statement, i, uuid);
        }
        else if (type == CASS_VALUE_TYPE_INT)
        {
            lua_Integer lua_integer = lua_tointeger(L, -1);
            err = cass_statement_bind_int32(statement, i, lua_integer);
        }
        else if (type == CASS_VALUE_TYPE_SMALL_INT)
        {
            err = cass_statement_bind_int16(statement, i, lua_tointeger(L, -1));
        }
        else if (type == CASS_VALUE_TYPE_TINY_INT)
        {
            err = cass_statement_bind_int8(statement, i, lua_tointeger(L, -1));
        }
        else if (type == CASS_VALUE_TYPE_BIGINT)
        {
            err = cass_statement_bind_int64(statement, i, lua_tointeger(L, -1));
        }
        else if (type == CASS_VALUE_TYPE_DOUBLE)
        {
            err = cass_statement_bind_double(statement, i, lua_tonumber(L, -1));
        }
        else if (type == CASS_VALUE_TYPE_VARCHAR)
        {
            err = cass_statement_bind_string(statement, i, lua_tostring(L, -1));
        }
        else
        {
            error_to_lua(L, "invalid type: %d", type);
        }

        if (err != CASS_OK)
        {
            error_cass_to_lua(L, err, "error binding: %d", type);
        }

        lua_pop(L, 2);
    }
    return statement;
}

static int query(lua_State *L)
{
    cass_log_set_level(CASS_LOG_DEBUG);
    luaL_checktype(L, QUERY_POSITION, LUA_TSTRING);
    luaL_checktype(L, PARAMETERS_POSITION, LUA_TTABLE);
    CassError err;

    // Create statement
    CassStatement *statement = create_statement(L);

    CassFuture *execute_future = cass_session_execute(session, statement);
    const CassResult *execute_result = cass_future_get_result(execute_future);
    CassIterator *iterator = cass_iterator_from_result(execute_result);
    size_t row_count = cass_result_row_count(execute_result);
    size_t col_count = cass_result_column_count(execute_result);

    lua_newtable(L);
    int main_table = lua_gettop(L);
    int table_index = 1;

    while (cass_iterator_next(iterator) == cass_true)
    {
        const CassRow *row = cass_iterator_get_row(iterator);

        lua_pushinteger(L, table_index++);
        lua_newtable(L);
        int sub_table = lua_gettop(L);

        for (size_t c = 0; c < col_count; c++)
        {
            const CassValue *cass_value = cass_row_get_column(row, c);

            const char *col_name;
            size_t col_name_len;
            cass_result_column_name(execute_result, c, &col_name, &col_name_len);
            char terminated[col_name_len + 1];
            strncpy(terminated, col_name, col_name_len + 1);

            const CassDataType *dt = cass_result_column_data_type(execute_result, c);
            const CassValueType vt = cass_data_type_type(dt);

            const char *str_val;
            size_t str_len;
            char uuid_str_val[CASS_UUID_STRING_LENGTH];
            CassUuid uuid_val;
            cass_int32_t int32_val;
            cass_bool_t bool_val;
            cass_double_t double_val;

            switch (vt)
            {
            case CASS_VALUE_TYPE_VARCHAR:
                cass_value_get_string(cass_value, &str_val, &str_len);
                lua_pushstring(L, terminated);
                lua_pushstring(L, str_val);
                break;
            case CASS_VALUE_TYPE_UUID:
            case CASS_VALUE_TYPE_TIMEUUID:
                cass_value_get_uuid(cass_value, &uuid_val);
                cass_uuid_string(uuid_val, uuid_str_val);
                lua_pushstring(L, terminated);
                lua_pushstring(L, uuid_str_val);
                break;
            case CASS_VALUE_TYPE_SMALL_INT:
            case CASS_VALUE_TYPE_TINY_INT:
            case CASS_VALUE_TYPE_BIGINT:
                break;
            case CASS_VALUE_TYPE_INT:
                cass_value_get_int32(cass_value, &int32_val);
                lua_pushstring(L, terminated);
                lua_pushinteger(L, int32_val);
                break;
            case CASS_VALUE_TYPE_BOOLEAN:
                cass_value_get_bool(cass_value, &bool_val);
                lua_pushstring(L, terminated);
                lua_pushboolean(L, bool_val);
                break;
            case CASS_VALUE_TYPE_DOUBLE:
                cass_value_get_double(cass_value, &double_val);
                lua_pushstring(L, terminated);
                lua_pushnumber(L, double_val);
                break;
            default:
                lua_pushstring(L, terminated);
                lua_pushnil(L);
                break;
            }
            lua_settable(L, sub_table);
        }
        lua_settable(L, main_table);
    }
    return 1;
}

int luaopen_luacassandra(lua_State *L)
{
    cass_log_set_level(CASS_LOG_DISABLED);
    luaL_Reg reg[] = {
        {"connect", connect},
        {"query", query},
        {NULL, NULL},
    };
    luaL_openlib(L, "luacassandra", reg, 0);
    return 1;
}

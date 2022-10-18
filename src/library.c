#include "library.h"
#include "cassandra.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CassSession *session;
CassCluster *cluster;

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
        lua_pushstring(L, "could not set contact points");
        lua_error(L);
    }
    CassFuture *future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    cass_future_free(future);
    if (err != CASS_OK)
    {
        lua_pushstring(L, "could not connect");
        lua_error(L);
    }
    lua_newtable(L);
    return 0;
}

void bind_number(CassStatement *statement, size_t index, CassValueType cass_type, lua_Number value)
{
    CassError err = cass_statement_bind_int32(statement, index, value);
    if (err != CASS_OK)
    {
        const char *desc = cass_error_desc(err);
        fprintf(stderr, "error: %s\n", desc);
    }
}

static int query(lua_State *L)
{
    const int QUERY_POSITION = 1;
    const int PARAMETERS_POSITION = 2;
    const int TYPE_POSITION = 1;
    const int VALUE_POSITION = 2;
    cass_log_set_level(CASS_LOG_DEBUG);
    luaL_checktype(L, QUERY_POSITION, LUA_TSTRING);
    luaL_checktype(L, PARAMETERS_POSITION, LUA_TTABLE);

    // Create statement
    const size_t param_count = lua_objlen(L, 2);
    const char *query = lua_tostring(L, QUERY_POSITION);
    printf("creating statement `%s` with %ld parameter(s)\n", query, param_count);
    CassStatement *statement = cass_statement_new(query, param_count);

    for (size_t i = 0; i < param_count; i++)
    {
        lua_rawgeti(L, PARAMETERS_POSITION, i + 1);
        int current = lua_gettop(L);
        lua_rawgeti(L, current, TYPE_POSITION);
        lua_rawgeti(L, current, VALUE_POSITION);
        const char *type = lua_tostring(L, -2);
        const lua_Integer value = lua_tointeger(L, -1);
        printf("type=%s value=%ld\n", type, value);

        bind_number(statement, i, CASS_VALUE_TYPE_INT, value);
        lua_pop(L, 2);
    }

    CassFuture *execute_future = cass_session_execute(session, statement);
    cass_future_wait(execute_future);
    printf("done executing\n");
    const CassResult *execute_result = cass_future_get_result(execute_future);
    CassIterator *iterator = cass_iterator_from_result(execute_result);
    size_t row_count = cass_result_row_count(execute_result);
    size_t col_count = cass_result_column_count(execute_result);
    printf("results: rows=%ld, cols=%ld\n", row_count, col_count);

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
            CassError err;

            switch (vt)
            {
            case CASS_VALUE_TYPE_VARCHAR:
                cass_value_get_string(cass_value, &str_val, &str_len);
                lua_pushstring(L, terminated);
                lua_pushstring(L, str_val);
                lua_settable(L, sub_table);
                continue;
            case CASS_VALUE_TYPE_UUID:
            case CASS_VALUE_TYPE_TIMEUUID:
                cass_value_get_uuid(cass_value, &uuid_val);
                cass_uuid_string(uuid_val, uuid_str_val);
                lua_pushstring(L, terminated);
                lua_pushstring(L, uuid_str_val);
                lua_settable(L, sub_table);
                continue;
            case CASS_VALUE_TYPE_SMALL_INT:
            case CASS_VALUE_TYPE_TINY_INT:
            case CASS_VALUE_TYPE_BIGINT:
                continue;
            case CASS_VALUE_TYPE_INT:
                cass_value_get_int32(cass_value, &int32_val);
                lua_pushstring(L, terminated);
                lua_pushinteger(L, int32_val);
                lua_settable(L, sub_table);
                continue;
            case CASS_VALUE_TYPE_BOOLEAN:
                cass_value_get_bool(cass_value, &bool_val);
                lua_pushstring(L, terminated);
                lua_pushboolean(L, bool_val);
                lua_settable(L, sub_table);
                continue;
            case CASS_VALUE_TYPE_DOUBLE:
                cass_value_get_double(cass_value, &double_val);
                lua_pushstring(L, terminated);
                lua_pushnumber(L, double_val);
                lua_settable(L, sub_table);
                continue;
            default:
                printf("could not find type for column: %s\n", terminated);
                lua_pushstring(L, terminated);
                lua_pushnil(L);
                lua_settable(L, sub_table);
                continue;
            }
        }
        lua_settable(L, main_table);
    }
    return 1;
}

int luaopen_luacassandra(lua_State *L)
{
    printf("registering luacassandra\n");
    luaL_Reg reg[] = {
        {"connect", connect},
        {"query", query},
        {NULL, NULL},
    };
    luaL_openlib(L, "luacassandra", reg, 0);
    return 1;
}

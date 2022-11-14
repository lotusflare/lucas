#include "cassandra.h"
#include "errors.c"
#include "logging.c"
#include "state.c"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CassCollection *create_map(lua_State *L, int index, CassStatement *statement)
{
    size_t map_size = lua_objlen(L, lua_gettop(L));
    CassCollection *map = cass_collection_new(CASS_COLLECTION_TYPE_MAP, map_size);
    lua_pushnil(L);

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int key_type = lua_type(L, key_index);
        const int value_index = lua_gettop(L);
        const int value_type = lua_type(L, value_index);
        CassError err;

        if (key_type == LUA_TSTRING)
        {
            err = cass_collection_append_string(map, lua_tostring(L, key_index));
        }
        else if (key_type == LUA_TNUMBER)
        {
            err = cass_collection_append_int32(map, lua_tointeger(L, key_index));
        }
        else if (key_type == LUA_TBOOLEAN)
        {
            err = cass_collection_append_bool(map, lua_toboolean(L, key_index));
        }

        if (value_type == LUA_TSTRING)
        {
            err = cass_collection_append_string(map, lua_tostring(L, value_index));
        }
        else if (value_type == LUA_TNUMBER)
        {
            err = cass_collection_append_int32(map, lua_tointeger(L, value_index));
        }
        else if (value_type == LUA_TBOOLEAN)
        {
            err = cass_collection_append_bool(map, lua_toboolean(L, value_index));
        }

        if (err != CASS_OK)
        {
            errorf_cass_to_lua(L, err, "could not append to map");
        }
    }

    return map;
}

CassCollection *create_collection(lua_State *L, int index, CassStatement *statement, CassCollectionType collection_type)
{
    size_t collection_size = lua_objlen(L, lua_gettop(L));
    CassCollection *collection = cass_collection_new(collection_type, collection_size);
    lua_pushnil(L);

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int value_index = lua_gettop(L);
        const int value_type = lua_type(L, value_index);
        CassError err;

        if (value_type == LUA_TSTRING)
        {
            err = cass_collection_append_string(collection, lua_tostring(L, value_index));
        }
        else if (value_type == LUA_TNUMBER)
        {
            err = cass_collection_append_int32(collection, lua_tointeger(L, value_index));
        }
        else if (value_type == LUA_TBOOLEAN)
        {
            err = cass_collection_append_bool(collection, lua_toboolean(L, value_index));
        }

        if (err != CASS_OK)
        {
            errorf_cass_to_lua(L, err, "could not append to collection");
        }
    }

    return collection;
}

void bind_positional_parameter(lua_State *L, int i, CassStatement *statement, CassValueType type, int index)
{
    CassError err;

    if (type == CASS_VALUE_TYPE_UUID || type == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid uuid;
        err = cass_uuid_from_string(lua_tostring(L, index), &uuid);
        if (err == CASS_OK)
        {
            cass_statement_bind_uuid(statement, i, uuid);
        }
    }
    else if (type == CASS_VALUE_TYPE_BOOLEAN)
    {
        cass_statement_bind_bool(statement, i, lua_toboolean(L, index));
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
    else if (type == CASS_VALUE_TYPE_BIGINT || type == CASS_VALUE_TYPE_TIMESTAMP)
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
    else if (type == CASS_VALUE_TYPE_MAP)
    {
        CassCollection *map = create_map(L, index, statement);
        cass_statement_bind_collection(statement, i, map);
    }
    else if (type == CASS_VALUE_TYPE_LIST)
    {
        CassCollection *collection = create_collection(L, index, statement, CASS_COLLECTION_TYPE_LIST);
        cass_statement_bind_collection(statement, i, collection);
    }
    else if (type == CASS_VALUE_TYPE_SET)
    {
        CassCollection *collection = create_collection(L, index, statement, CASS_COLLECTION_TYPE_SET);
        cass_statement_bind_collection(statement, i, collection);
    }
    else if (type == CASS_VALUE_TYPE_NULL)
    {
        err = cass_statement_bind_null(statement, i);
    }
    else if (type == CASS_VALUE_TYPE_UNSET)
    {
        return;
    }
    else
    {
        errorf_to_lua(L, "invalid type %d for parameter %d", type, i);
    }

    if (err != CASS_OK)
    {
        errorf_cass_to_lua(L, err, "could not bind positional parameter at index %d", i);
    }
}

void bind_named_parameter(lua_State *L, const char *name, CassStatement *statement, CassValueType type, int index)
{
    CassError err;

    if (type == CASS_VALUE_TYPE_UUID || type == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid uuid;
        err = cass_uuid_from_string(lua_tostring(L, index), &uuid);
        if (err == CASS_OK)
        {
            cass_statement_bind_uuid_by_name(statement, name, uuid);
        }
    }
    else if (type == CASS_VALUE_TYPE_BOOLEAN)
    {
        cass_statement_bind_bool_by_name(statement, name, lua_toboolean(L, index));
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
    else if (type == CASS_VALUE_TYPE_BIGINT || type == CASS_VALUE_TYPE_TIMESTAMP)
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
    else if (type == CASS_VALUE_TYPE_MAP)
    {
        CassCollection *map = create_map(L, index, statement);
        err = cass_statement_bind_collection_by_name(statement, name, map);
    }
    else if (type == CASS_VALUE_TYPE_LIST)
    {
        CassCollection *collection = create_collection(L, index, statement, CASS_COLLECTION_TYPE_LIST);
        err = cass_statement_bind_collection_by_name(statement, name, collection);
    }
    else if (type == CASS_VALUE_TYPE_SET)
    {
        CassCollection *collection = create_collection(L, index, statement, CASS_COLLECTION_TYPE_SET);
        err = cass_statement_bind_collection_by_name(statement, name, collection);
    }
    else if (type == CASS_VALUE_TYPE_NULL)
    {
        err = cass_statement_bind_null_by_name(statement, name);
    }
    else if (type == CASS_VALUE_TYPE_UNSET)
    {
        return;
    }
    else
    {
        errorf_to_lua(L, "invalid type %d for parameter %s", type, name);
    }

    if (err != CASS_OK)
    {
        errorf_cass_to_lua(L, err, "could not bind named parameter %s", name);
    }
}

void bind_parameters(lua_State *L, int index, CassStatement *statement)
{
    lua_pushnil(L);

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int key_type = lua_type(L, key_index);
        const int tuple_index = lua_gettop(L);
        lua_pushvalue(L, key_index); // copy the key
        const int key_copy_index = lua_gettop(L);

        lua_rawgeti(L, tuple_index, 1);
        lua_rawgeti(L, tuple_index, 2);
        const int type_index = lua_gettop(L) - 1;
        const int value_index = lua_gettop(L);
        const CassValueType type = lua_tointeger(L, type_index);

        if (key_type == LUA_TSTRING)
        {
            const char *name = lua_tostring(L, key_copy_index);
            bind_named_parameter(L, name, statement, type, value_index);
        }
        else if (key_type == LUA_TNUMBER)
        {
            const int index = lua_tointeger(L, key_copy_index) - 1; // lua base 1 indexing
            bind_positional_parameter(L, index, statement, type, value_index);
        }
        else
        {
            errorf_to_lua(L, "invalid key type %s for parameter", lua_typename(L, key_index));
        }
    }
}

void cass_value_to_lua(lua_State *L, const CassValue *cass_value)
{
    CassValueType vt = cass_value_type(cass_value);

    if (cass_value_is_null(cass_value) || vt == CASS_VALUE_TYPE_NULL || vt == CASS_VALUE_TYPE_UNSET)
    {
        lua_pushnil(L);
    }
    else if (vt == CASS_VALUE_TYPE_ASCII || vt == CASS_VALUE_TYPE_TEXT || vt == CASS_VALUE_TYPE_VARCHAR)
    {
        const char *value;
        size_t length;
        cass_value_get_string(cass_value, &value, &length);
        lua_pushlstring(L, value, length);
    }
    else if (vt == CASS_VALUE_TYPE_UUID || vt == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid value;
        char value_as_string[CASS_UUID_STRING_LENGTH];
        cass_value_get_uuid(cass_value, &value);
        cass_uuid_string(value, value_as_string);
        lua_pushstring(L, value_as_string);
    }
    else if (vt == CASS_VALUE_TYPE_TINY_INT)
    {
        cass_int8_t value;
        cass_value_get_int8(cass_value, &value);
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_SMALL_INT)
    {
        cass_int16_t value;
        cass_value_get_int16(cass_value, &value);
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_INT)
    {
        cass_int32_t value;
        cass_value_get_int32(cass_value, &value);
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_BIGINT || vt == CASS_VALUE_TYPE_TIMESTAMP)
    {
        cass_int64_t value;
        cass_value_get_int64(cass_value, &value);
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_BOOLEAN)
    {
        cass_bool_t value;
        cass_value_get_bool(cass_value, &value);
        lua_pushboolean(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_FLOAT)
    {
        cass_float_t value;
        cass_value_get_float(cass_value, &value);
        lua_pushnumber(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_DOUBLE)
    {
        cass_double_t value;
        cass_value_get_double(cass_value, &value);
        lua_pushnumber(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_MAP)
    {
        CassIterator *iterator = cass_iterator_from_map(cass_value);
        lua_newtable(L);
        int map_table = lua_gettop(L);

        while (cass_iterator_next(iterator))
        {
            cass_value_to_lua(L, cass_iterator_get_map_key(iterator));
            cass_value_to_lua(L, cass_iterator_get_map_value(iterator));
            lua_settable(L, map_table);
        }
        cass_iterator_free(iterator);
    }
    else if (vt == CASS_VALUE_TYPE_LIST || vt == CASS_VALUE_TYPE_SET)
    {
        CassIterator *iterator = cass_iterator_from_collection(cass_value);
        lua_newtable(L);
        int list_table = lua_gettop(L);

        for (int i = 1; cass_iterator_next(iterator); i++)
        {
            lua_pushinteger(L, i);
            cass_value_to_lua(L, cass_iterator_get_value(iterator));
            lua_settable(L, list_table);
        }
        cass_iterator_free(iterator);
    }
    else
    {
        errorf_to_lua(L, "unrecognized cassandra type");
    }
}

void iterate_result(lua_State *L, CassStatement *statement, const char *paging_state, size_t paging_state_size)
{
    if (paging_state != NULL)
    {
        cass_statement_set_paging_state_token(statement, paging_state, paging_state_size);
    }

    CassFuture *future = cass_session_execute(session, statement);
    cass_future_wait(future);
    if (cass_future_error_code(future) != CASS_OK)
    {
        errorf_cass_future_to_lua(L, future, "execution of query failed");
    }

    const CassResult *result = cass_future_get_result(future);
    CassIterator *iterator = cass_iterator_from_result(result);
    size_t col_count = cass_result_column_count(result);

    lua_newtable(L);
    const int root_table = lua_gettop(L);

    for (int i = 1; cass_iterator_next(iterator); i++)
    {
        lua_pushinteger(L, i);
        lua_newtable(L);
        int sub_table = lua_gettop(L);
        const CassRow *row = cass_iterator_get_row(iterator);
        for (size_t c = 0; c < col_count; c++)
        {
            const char *name;
            size_t name_length;
            cass_result_column_name(result, c, &name, &name_length);
            lua_pushlstring(L, name, name_length);
            const CassValue *cass_value = cass_row_get_column(row, c);
            cass_value_to_lua(L, cass_value);
            lua_settable(L, sub_table);
        }
        lua_settable(L, root_table);
    }

    lua_newtable(L);
    const int meta_table = lua_gettop(L);
    if (cass_result_has_more_pages(result))
    {
        const char *paging_state;
        size_t paging_state_size;
        CassError err = cass_result_paging_state_token(result, &paging_state, &paging_state_size);
        if (err != CASS_OK)
        {
            errorf_cass_to_lua(L, err, "could not get paging state token");
        }
        err = cass_statement_set_paging_state_token(statement, paging_state, paging_state_size);
        if (err != CASS_OK)
        {
            errorf_cass_to_lua(L, err, "could not set paging state token");
        }
        lua_pushstring(L, "has_more_pages");
        lua_pushboolean(L, 1);
        lua_settable(L, meta_table);
        lua_pushstring(L, "paging_state");
        lua_pushlstring(L, paging_state, paging_state_size);
        lua_settable(L, meta_table);
    }

    cass_result_free(result);
    cass_iterator_free(iterator);
    cass_future_free(future);
}

CassStatement *create_prepared_statement(lua_State *L, const char *query)
{
    CassFuture *future = cass_session_prepare(session, query);
    cass_future_wait(future);
    if (cass_future_error_code(future) != CASS_OK)
    {
        errorf_cass_future_to_lua(L, future, "failed to create prepared statement");
    }
    CassStatement *statement = cass_prepared_bind(cass_future_get_prepared(future));
    cass_future_free(future);
    return statement;
}

static int query(lua_State *L)
{
    lucas_log(CASS_LOG_INFO, "Attempting to query");
    const int ARG_QUERY = 1;
    const int ARG_QUERY_PARAMS = 2;
    const int ARG_OPTIONS = 3;
    luaL_checktype(L, ARG_QUERY, LUA_TSTRING);
    luaL_checktype(L, ARG_QUERY_PARAMS, LUA_TTABLE);
    const size_t parameter_count = lua_objlen(L, ARG_QUERY_PARAMS);
    const char *query = lua_tostring(L, ARG_QUERY);
    size_t paging_state_size = 0;
    const char *paging_state = NULL;
    int page_size = 0;

    if (lua_type(L, ARG_OPTIONS) == LUA_TTABLE)
    {
        lua_getfield(L, ARG_OPTIONS, "paging_state");
        paging_state = lua_tolstring(L, lua_gettop(L), &paging_state_size);
        lua_getfield(L, ARG_OPTIONS, "page_size");
        page_size = lua_tointeger(L, lua_gettop(L));
    }

    if (page_size == 0)
    {
        page_size = 500;
    }

    if (session == NULL)
    {
        errorf_to_lua(L, "not connected");
    }

    CassStatement *statement = create_prepared_statement(L, query);
    cass_statement_set_paging_size(statement, page_size);
    bind_parameters(L, ARG_QUERY_PARAMS, statement);
    iterate_result(L, statement, paging_state, paging_state_size);
    cass_statement_free(statement);

    return 2;
}

void prepare_insert_into_batch(lua_State *L, CassSession *session, const char *query, const CassPrepared **prepared)
{
    CassFuture *future = cass_session_prepare(session, query);
    cass_future_wait(future);
    CassError err = cass_future_error_code(future);
    if (err != CASS_OK)
    {
        errorf_cass_future_to_lua(L, future, "could not create prepared statement");
    }
    *prepared = cass_future_get_prepared(future);
    cass_future_free(future);
}

int batch(lua_State *L)
{
    const int ARG_QUERY = 1;
    const int ARG_BATCHES = 2;
    const size_t parameter_count = lua_objlen(L, ARG_BATCHES);
    const char *query = lua_tostring(L, ARG_QUERY);
    CassBatch *batch = cass_batch_new(CASS_BATCH_TYPE_UNLOGGED);
    const CassPrepared *prepared = NULL;
    prepare_insert_into_batch(L, session, query, &prepared);

    lua_pushnil(L);
    for (int last_top = lua_gettop(L); lua_next(L, ARG_BATCHES) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        CassStatement *statement = cass_prepared_bind(prepared);
        bind_parameters(L, lua_gettop(L), statement);
        CassError err = cass_batch_add_statement(batch, statement);
        cass_statement_free(statement);
        if (err != CASS_OK)
        {
            errorf_cass_to_lua(L, err, "failed to add statement to batch");
        }
    }

    CassFuture *future = cass_session_execute_batch(session, batch);
    cass_future_wait(future);
    cass_prepared_free(prepared);
    CassError err = cass_future_error_code(future);
    cass_future_free(future);
    if (err != CASS_OK)
    {
        errorf_cass_future_to_lua(L, future, "executing batch query failed");
    }

    return 0;
}

#pragma once

#include "query.h"
#include "cassandra.h"
#include "errors.c"
#include "logs.c"
#include "state.c"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LucasError *append_collection(lua_State *L, int index, CassCollection *collection)
{
    CassError err = CASS_OK;
    LucasError *rc = NULL;
    lua_rawgeti(L, index, 1);
    lua_rawgeti(L, index, 2);
    const int type_index = lua_gettop(L) - 1;
    const int value_index = lua_gettop(L);
    const CassValueType type = lua_tointeger(L, type_index);

    if (type == CASS_VALUE_TYPE_UUID || type == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid uuid;
        err = cass_uuid_from_string(lua_tostring(L, value_index), &uuid);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        err = cass_collection_append_uuid(collection, uuid);
    }
    else if (type == CASS_VALUE_TYPE_BOOLEAN)
    {
        err = cass_collection_append_bool(collection, lua_toboolean(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_TINY_INT)
    {
        err = cass_collection_append_int8(collection, lua_tointeger(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_SMALL_INT)
    {
        err = cass_collection_append_int16(collection, lua_tointeger(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_INT)
    {
        err = cass_collection_append_int32(collection, lua_tointeger(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_BIGINT || type == CASS_VALUE_TYPE_TIMESTAMP || type == CASS_VALUE_TYPE_COUNTER ||
             type == CASS_VALUE_TYPE_TIME)
    {
        err = cass_collection_append_int64(collection, lua_tointeger(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_FLOAT)
    {
        err = cass_collection_append_float(collection, lua_tonumber(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_DOUBLE)
    {
        err = cass_collection_append_double(collection, lua_tonumber(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_ASCII || type == CASS_VALUE_TYPE_TEXT || type == CASS_VALUE_TYPE_VARCHAR)
    {
        err = cass_collection_append_string(collection, lua_tostring(L, value_index));
    }
    else if (type == CASS_VALUE_TYPE_MAP)
    {
        rc = create_map(L, value_index, &collection);
    }
    else if (type == CASS_VALUE_TYPE_LIST)
    {
        rc = create_collection(L, value_index, CASS_COLLECTION_TYPE_LIST, &collection);
    }
    else if (type == CASS_VALUE_TYPE_SET)
    {
        rc = create_collection(L, value_index, CASS_COLLECTION_TYPE_SET, &collection);
    }
    else
    {
        rc = lucas_new_errorf("invalid type to append to collection");
    }

cleanup:
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not append to collection");
    }
    return rc;
}

LucasError *create_map(lua_State *L, int index, CassCollection **collection)
{
    CassError err = CASS_OK;
    LucasError *rc = NULL;
    *collection = cass_collection_new(CASS_COLLECTION_TYPE_MAP, 0);
    lua_pushnil(L);

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_settop(L, last_top))
    {
        const int key_index = lua_gettop(L) - 1;
        const int value_index = lua_gettop(L);
        rc = append_collection(L, key_index, *collection);
        if (rc)
        {
            return lucas_wrap_error(rc, "failed adding collection key");
        }
        rc = append_collection(L, value_index, *collection);
        if (rc)
        {
            return lucas_wrap_error(rc, "failed adding collection value");
        }
    }

cleanup:
    return rc;
}

LucasError *create_collection(lua_State *L, int index, CassCollectionType type, CassCollection **collection)
{
    CassError err = CASS_OK;
    LucasError *rc = NULL;
    size_t size = lua_objlen(L, lua_gettop(L));
    *collection = cass_collection_new(type, size);
    lua_pushnil(L);

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_settop(L, last_top))
    {
        const int value_index = lua_gettop(L);
        rc = append_collection(L, value_index, *collection);
        if (rc)
        {
            return lucas_wrap_error(rc, "could not add element to collection");
        }
    }

    return rc;
}

LucasError *bind_positional_parameter(lua_State *L, int i, CassStatement *statement, CassValueType type, int index)
{
    LucasError *rc = NULL;
    CassError err = CASS_OK;
    CassCollection *collection = NULL;

    if (type == CASS_VALUE_TYPE_UUID || type == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid uuid;
        err = cass_uuid_from_string(lua_tostring(L, index), &uuid);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        err = cass_statement_bind_uuid(statement, i, uuid);
    }
    else if (type == CASS_VALUE_TYPE_BOOLEAN)
    {
        err = cass_statement_bind_bool(statement, i, lua_toboolean(L, index));
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
    else if (type == CASS_VALUE_TYPE_BIGINT || type == CASS_VALUE_TYPE_TIMESTAMP || type == CASS_VALUE_TYPE_COUNTER ||
             type == CASS_VALUE_TYPE_TIME)
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
        rc = create_map(L, index, &collection);
        if (rc)
        {
            goto cleanup;
        }
        err = cass_statement_bind_collection(statement, i, collection);
    }
    else if (type == CASS_VALUE_TYPE_LIST)
    {
        rc = create_collection(L, index, CASS_COLLECTION_TYPE_LIST, &collection);
        if (rc)
        {
            goto cleanup;
        }
        err = cass_statement_bind_collection(statement, i, collection);
    }
    else if (type == CASS_VALUE_TYPE_SET)
    {
        rc = create_collection(L, index, CASS_COLLECTION_TYPE_SET, &collection);
        if (rc)
        {
            goto cleanup;
        }
        err = cass_statement_bind_collection(statement, i, collection);
    }
    else if (type == CASS_VALUE_TYPE_NULL)
    {
        err = cass_statement_bind_null(statement, i);
    }
    else if (type == CASS_VALUE_TYPE_UNSET)
    {
        return NULL;
    }
    else
    {
        rc = lucas_new_errorf("invalid type %d for parameter %d", type, i);
    }

cleanup:
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not bind positional parameter at index %d", i);
    }
    if (collection)
    {
        cass_collection_free(collection);
    }
    return rc;
}

LucasError *bind_named_parameter(lua_State *L, const char *name, CassStatement *statement, CassValueType type,
                                 int index)
{
    LucasError *rc = NULL;
    CassError err = CASS_OK;
    CassCollection *collection = NULL;

    if (type == CASS_VALUE_TYPE_UUID || type == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid uuid;
        err = cass_uuid_from_string(lua_tostring(L, index), &uuid);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        err = cass_statement_bind_uuid_by_name(statement, name, uuid);
    }
    else if (type == CASS_VALUE_TYPE_BOOLEAN)
    {
        err = cass_statement_bind_bool_by_name(statement, name, lua_toboolean(L, index));
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
    else if (type == CASS_VALUE_TYPE_BIGINT || type == CASS_VALUE_TYPE_TIMESTAMP || type == CASS_VALUE_TYPE_COUNTER ||
             type == CASS_VALUE_TYPE_TIME)
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
        rc = create_map(L, index, &collection);
        if (rc)
        {
            goto cleanup;
        }
        err = cass_statement_bind_collection_by_name(statement, name, collection);
    }
    else if (type == CASS_VALUE_TYPE_LIST)
    {
        rc = create_collection(L, index, CASS_COLLECTION_TYPE_LIST, &collection);
        if (rc)
        {
            goto cleanup;
        }
        err = cass_statement_bind_collection_by_name(statement, name, collection);
    }
    else if (type == CASS_VALUE_TYPE_SET)
    {
        rc = create_collection(L, index, CASS_COLLECTION_TYPE_SET, &collection);
        if (rc)
        {
            goto cleanup;
        }
        err = cass_statement_bind_collection_by_name(statement, name, collection);
    }
    else if (type == CASS_VALUE_TYPE_NULL)
    {
        err = cass_statement_bind_null_by_name(statement, name);
    }
    else if (type == CASS_VALUE_TYPE_UNSET)
    {
        return NULL;
    }
    else
    {
        rc = lucas_new_errorf("invalid type %d for parameter %s", type, name);
    }

cleanup:
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not bind named parameter %s", name);
    }
    return rc;
}

LucasError *bind_parameters(lua_State *L, int index, CassStatement *statement)
{
    LucasError *rc = NULL;
    lua_pushnil(L);

    for (int last_top = lua_gettop(L); lua_next(L, index) != 0; lua_settop(L, last_top))
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
            rc = bind_named_parameter(L, name, statement, type, value_index);
            if (rc)
            {
                goto cleanup;
            }
        }
        else if (key_type == LUA_TNUMBER)
        {
            const int index = lua_tointeger(L, key_copy_index) - 1; // lua base 1 indexing
            rc = bind_positional_parameter(L, index, statement, type, value_index);
            if (rc)
            {
                goto cleanup;
            }
        }
        else
        {
            rc = lucas_new_errorf("invalid key type %s for parameter", lua_typename(L, key_index));
            goto cleanup;
        }
    }

cleanup:
    return rc;
}

LucasError *cass_value_to_lua(lua_State *L, const CassValue *cass_value)
{
    CassValueType vt = cass_value_type(cass_value);
    CassError err = CASS_OK;
    LucasError *rc = NULL;
    CassIterator *iterator = NULL;

    if (cass_value_is_null(cass_value) || vt == CASS_VALUE_TYPE_NULL || vt == CASS_VALUE_TYPE_UNSET)
    {
        lua_pushnil(L);
    }
    else if (vt == CASS_VALUE_TYPE_ASCII || vt == CASS_VALUE_TYPE_TEXT || vt == CASS_VALUE_TYPE_VARCHAR)
    {
        const char *value;
        size_t length;
        err = cass_value_get_string(cass_value, &value, &length);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushlstring(L, value, length);
    }
    else if (vt == CASS_VALUE_TYPE_UUID || vt == CASS_VALUE_TYPE_TIMEUUID)
    {
        CassUuid value;
        char value_as_string[CASS_UUID_STRING_LENGTH];
        err = cass_value_get_uuid(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        cass_uuid_string(value, value_as_string);
        lua_pushstring(L, value_as_string);
    }
    else if (vt == CASS_VALUE_TYPE_TINY_INT)
    {
        cass_int8_t value;
        err = cass_value_get_int8(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_SMALL_INT)
    {
        cass_int16_t value;
        err = cass_value_get_int16(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_INT)
    {
        cass_int32_t value;
        err = cass_value_get_int32(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_BIGINT || vt == CASS_VALUE_TYPE_TIMESTAMP || vt == CASS_VALUE_TYPE_COUNTER ||
             vt == CASS_VALUE_TYPE_TIME)
    {
        cass_int64_t value;
        err = cass_value_get_int64(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushinteger(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_BOOLEAN)
    {
        cass_bool_t value;
        err = cass_value_get_bool(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushboolean(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_FLOAT)
    {
        cass_float_t value;
        err = cass_value_get_float(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushnumber(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_DOUBLE)
    {
        cass_double_t value;
        err = cass_value_get_double(cass_value, &value);
        if (err != CASS_OK)
        {
            goto cleanup;
        }
        lua_pushnumber(L, value);
    }
    else if (vt == CASS_VALUE_TYPE_MAP)
    {
        iterator = cass_iterator_from_map(cass_value);
        lua_newtable(L);
        int map_table = lua_gettop(L);
        while (cass_iterator_next(iterator))
        {
            rc = cass_value_to_lua(L, cass_iterator_get_map_key(iterator));
            if (rc)
            {
                rc = lucas_wrap_error(rc, "unable to convert cassandra map key to lua type");
                goto cleanup;
            }
            rc = cass_value_to_lua(L, cass_iterator_get_map_value(iterator));
            if (rc)
            {
                rc = lucas_wrap_error(rc, "unable to convert cassandra map value to lua type");
                goto cleanup;
            }
            lua_settable(L, map_table);
        }
    }
    else if (vt == CASS_VALUE_TYPE_LIST || vt == CASS_VALUE_TYPE_SET)
    {
        iterator = cass_iterator_from_collection(cass_value);
        lua_newtable(L);
        int list_table = lua_gettop(L);
        for (int i = 1; cass_iterator_next(iterator); i++)
        {
            rc = cass_value_to_lua(L, cass_iterator_get_value(iterator));
            if (rc)
            {
                rc = lucas_wrap_error(rc, "unable to convert cassandra list or set item to lua type");
                goto cleanup;
            }
            lua_rawseti(L, list_table, i);
        }
    }
    else
    {
        rc = lucas_new_errorf("unrecognized cassandra type");
    }

cleanup:
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not get value from cassandra");
    }
    if (iterator)
    {
        cass_iterator_free(iterator);
    }
    return rc;
}

LucasError *iterate_result(lua_State *L, CassStatement *statement, const char *paging_state, size_t paging_state_size)
{
    LucasError *rc = NULL;
    CassError err = CASS_OK;
    CassIterator *iterator = NULL;
    CassFuture *future = NULL;
    const CassResult *result = NULL;

    if (paging_state)
    {
        lucas_log(LOG_DEBUG, "setting paging token");
        err = cass_statement_set_paging_state_token(statement, paging_state, paging_state_size);
        if (err != CASS_OK)
        {
            rc = lucas_new_errorf_from_cass_error(err, "could not set paging state token");
            goto cleanup;
        }
    }

    future = cass_session_execute(session, statement);
    cass_future_wait(future);
    if (cass_future_error_code(future) != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_future(future, "execution of query failed");
        goto cleanup;
    }

    result = cass_future_get_result(future);
    iterator = cass_iterator_from_result(result);
    size_t col_count = cass_result_column_count(result);

    lua_newtable(L);
    const int root_table = lua_gettop(L);
    int counter = 1;
    lucas_log(LOG_DEBUG, "ready to query iterate results");

    while (cass_iterator_next(iterator))
    {
        lua_newtable(L);
        int sub_table = lua_gettop(L);
        const CassRow *row = cass_iterator_get_row(iterator);
        for (size_t c = 0; c < col_count; c++)
        {
            const char *name = NULL;
            size_t name_length = 0;
            err = cass_result_column_name(result, c, &name, &name_length);
            if (err != CASS_OK)
            {
                rc = lucas_new_errorf_from_cass_error(err, "could not get name of column");
                goto cleanup;
            }
            lua_pushlstring(L, name, name_length);
            rc = cass_value_to_lua(L, cass_row_get_column(row, c));
            if (rc)
            {
                rc = lucas_wrap_error(rc, "unable to convert cassandra value to lua");
                goto cleanup;
            }
            lua_settable(L, sub_table);
        }
        lua_rawseti(L, root_table, counter++);
    }

    lua_newtable(L);
    const int meta_table = lua_gettop(L);
    if (cass_result_has_more_pages(result))
    {
        lucas_log(LOG_DEBUG, "query result is paginated");
        const char *paging_state = NULL;
        size_t paging_state_size = 0;
        err = cass_result_paging_state_token(result, &paging_state, &paging_state_size);
        if (err != CASS_OK)
        {
            rc = lucas_new_errorf_from_cass_error(err, "could not get paging state token");
            goto cleanup;
        }
        err = cass_statement_set_paging_state_token(statement, paging_state, paging_state_size);
        if (err != CASS_OK)
        {
            rc = lucas_new_errorf_from_cass_error(err, "could not set paging state token");
            goto cleanup;
        }
        lua_pushstring(L, "has_more_pages");
        lua_pushboolean(L, true);
        lua_settable(L, meta_table);
        lua_pushstring(L, "paging_state");
        lua_pushlstring(L, paging_state, paging_state_size);
        lua_settable(L, meta_table);
        lucas_log(LOG_DEBUG, "finished setting paging state token");
    }

cleanup:
    if (iterator)
    {
        cass_iterator_free(iterator);
    }
    if (result)
    {
        cass_result_free(result);
    }
    if (future)
    {
        cass_future_free(future);
    }
    return rc;
}

LucasError *create_prepared_statement(lua_State *L, const char *query, CassStatement **statement)
{
    LucasError *rc = NULL;
    const CassPrepared *prepared = NULL;
    CassFuture *future = cass_session_prepare(session, query);
    cass_future_wait(future);
    CassError err = cass_future_error_code(future);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_future(future, "failed to create prepared statement");
        goto cleanup;
    }
    prepared = cass_future_get_prepared(future);
    *statement = cass_prepared_bind(prepared);

cleanup:
    if (future)
    {
        cass_future_free(future);
    }
    if (prepared)
    {
        cass_prepared_free(prepared);
    }
    return rc;
}

static int query(lua_State *L)
{
    const int ARG_QUERY = 1;
    const int ARG_QUERY_PARAMS = 2;
    const int ARG_OPTIONS = 3;
    luaL_checktype(L, ARG_QUERY, LUA_TSTRING);
    luaL_checktype(L, ARG_QUERY_PARAMS, LUA_TTABLE);
    const char *query = lua_tostring(L, ARG_QUERY);
    size_t paging_state_size = 0;
    const char *paging_state = NULL;
    int page_size = 0;
    LucasError *rc = NULL;
    CassError err = CASS_OK;
    CassStatement *statement = NULL;
    lucas_log(LOG_DEBUG, "query submitted: %s", query);

    if (lua_type(L, ARG_OPTIONS) == LUA_TTABLE)
    {
        lua_getfield(L, ARG_OPTIONS, "paging_state");
        paging_state = lua_tolstring(L, lua_gettop(L), &paging_state_size);
        lua_getfield(L, ARG_OPTIONS, "page_size");
        page_size = lua_tointeger(L, lua_gettop(L));
    }
    if (page_size == 0)
    {
        lucas_log(LOG_DEBUG, "page size not specified, defaulting to 500");
        page_size = 500;
    }
    if (session == NULL)
    {
        rc = lucas_new_errorf("session is not connected");
        goto cleanup;
    }
    rc = create_prepared_statement(L, query, &statement);
    if (rc)
    {
        rc = lucas_wrap_error(rc, "failed to create prepared statement");
        goto cleanup;
    }
    err = cass_statement_set_paging_size(statement, page_size);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set paging size to %d", page_size);
        goto cleanup;
    }
    rc = bind_parameters(L, ARG_QUERY_PARAMS, statement);
    if (rc)
    {
        rc = lucas_wrap_error(rc, "failed to bind parameters");
        goto cleanup;
    }
    rc = iterate_result(L, statement, paging_state, paging_state_size);
    if (rc)
    {
        rc = lucas_wrap_error(rc, "failed to handle query results");
        goto cleanup;
    }

cleanup:
    if (statement)
    {
        cass_statement_free(statement);
    }
    if (rc)
    {
        lucas_error_to_lua(L, rc);
    }
    return 2;
}

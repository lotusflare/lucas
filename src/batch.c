#include "cassandra.h"
#include "errors.c"
#include "logging.c"
#include "query.c"
#include "state.c"
#include "types.c"
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LucasError *prepare_insert_into_batch(lua_State *L, CassSession *session, const char *query,
                                      const CassPrepared **prepared)
{
    LucasError *rc = NULL;
    CassFuture *future = cass_session_prepare(session, query);
    cass_future_wait(future);
    CassError err = cass_future_error_code(future);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_future(future, "could not create prepared statement");
        goto cleanup;
    }
    *prepared = cass_future_get_prepared(future);

cleanup:
    if (future)
    {
        cass_future_free(future);
    }
    return rc;
}

int batch(lua_State *L)
{
    const int ARG_BATCHES = 1;
    const size_t parameter_count = lua_objlen(L, ARG_BATCHES);

    CassBatch *batch = NULL;
    const CassPrepared *prepared = NULL;
    CassStatement *statement = NULL;
    LucasError *rc = NULL;
    CassFuture *future = NULL;
    CassError err = CASS_OK;

    batch = cass_batch_new(CASS_BATCH_TYPE_UNLOGGED);
    lua_pushnil(L);
    for (int last_top = lua_gettop(L); lua_next(L, ARG_BATCHES) != 0; lua_pop(L, lua_gettop(L) - last_top))
    {
        const int tuple_index = lua_gettop(L);
        lua_rawgeti(L, tuple_index, 1);
        lua_rawgeti(L, tuple_index, 2);
        const int query_index = lua_gettop(L) - 1;
        const int params_index = lua_gettop(L);

        const char *query = lua_tostring(L, query_index);
        rc = prepare_insert_into_batch(L, session, query, &prepared);
        if (rc)
        {
            goto cleanup;
        }
        statement = cass_prepared_bind(prepared);
        rc = bind_parameters(L, params_index, statement);
        if (rc)
        {
            goto cleanup;
        }
        err = cass_batch_add_statement(batch, statement);
        if (err != CASS_OK)
        {
            rc = lucas_new_errorf_from_cass_error(err, "failed to add statement to batch");
            goto cleanup;
        }
    }

    future = cass_session_execute_batch(session, batch);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_future(future, "executing batch query failed");
        goto cleanup;
    }

cleanup:
    if (future)
    {
        cass_future_free(future);
    }
    if (batch)
    {
        cass_batch_free(batch);
    }
    if (statement)
    {
        cass_statement_free(statement);
    }
    if (rc)
    {
        lucas_error_to_lua(L, rc);
    }
    return 0;
}

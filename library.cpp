#include "include/library.h"
#include <iostream>
#include "luajit-2.1/lua.h"
#include "cassandra.h"

CassSession *session;
CassCluster *cluster;

const int QUERY_POSITION = 1;
const int PARAMETERS_POSITION = 2;

int connect(lua_State *L) {
    std::cout << "Hello, World!" << std::endl;
    CassError err;
    session = cass_session_new();
    cluster = cass_cluster_new();
    err = cass_cluster_set_contact_points(cluster, getenv("CASSANDRA_DB"));
    if (err != CASS_OK) {
        return 0;
    }
    CassFuture *future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    cass_future_free(future);
    if (err != CASS_OK) {
        return 0;
    }
}

int query(lua_State *L) {
    const int param_count = lua_objlen(L, 2);
    const char* query = lua_tostring(L, QUERY_POSITION);
    CassStatement *statement = cass_statement_new(query, param_count);
    CassFuture *future = cass_session_execute(session, statement);

    for (int i = 0; i < param_count; ++i) {
        lua_rawgeti(L, PARAMETERS_POSITION, i+1);
        int current = lua_gettop(L);
    }
}

int luaopen_helper(lua_State *L) {
    lua_newtable(L);
    
    lua_pushcfunction(L, connect);
    lua_setfield(L, -2, "connect");
    lua_pushcfunction(L, query);
    lua_setfield(L, -2, "query");
    
    return 1;
}

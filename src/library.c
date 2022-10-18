#include <stdio.h>
#include <stdlib.h>
#include "library.h"
#include "luajit-2.1/lua.h"
#include "luajit-2.1/lauxlib.h"
#include "cassandra.h"

CassSession *session;
CassCluster *cluster;

const int QUERY_POSITION = 1;
const int PARAMETERS_POSITION = 2;


int execute_query(lua_State *L) {}

void test() {
    printf("sdfsf\n");
}

static int connect(lua_State *L) {
    luaL_checkstring(L, 1);
    const char *contact_points = lua_tostring(L, 1);
    CassError err;
    session = cass_session_new();
    cluster = cass_cluster_new();
    err = cass_cluster_set_contact_points(cluster, contact_points);
    if (err != CASS_OK) {
        lua_pushstring(L, "could not set contact points");
        lua_error(L);
    }
    CassFuture *future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    cass_future_free(future);
    if (err != CASS_OK) {
        lua_pushstring(L, "could not connect");
        lua_error(L);
    }
    lua_newtable(L);
    return 0;
}

static int query(lua_State *L) {
    const int param_count = lua_objlen(L, 2);
    const char* query = lua_tostring(L, QUERY_POSITION);
    CassStatement *statement = cass_statement_new(query, param_count);
    CassFuture *future = cass_session_execute(session, statement);

    for (int i = 0; i < param_count; ++i) {
        lua_rawgeti(L, PARAMETERS_POSITION, i+1);
        int current = lua_gettop(L);
    }
}

int luaopen_luacassandra(lua_State *L) {
    printf("registering luacassandra\n");
    luaL_Reg reg[] = {
        {"connect", connect},
        {NULL, NULL}
    };
    luaL_openlib(L, "luacassandra", reg, 0);
    return 1;
}

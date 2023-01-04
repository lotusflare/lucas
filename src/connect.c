#include "cassandra.h"
#include "errors.c"
#include "logs.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include "types.c"

int get_port(lua_State *L, int i)
{
    lua_getfield(L, i, "port");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        return 9042;
    }
    return lua_tointeger(L, lua_gettop(L));
}

int get_num_threads_io(lua_State *L, int i)
{
    lua_getfield(L, i, "num_threads_io");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        return 1;
    }
    return lua_tointeger(L, lua_gettop(L));
}

int get_connect_timeout(lua_State *L, int i)
{
    lua_getfield(L, i, "connect_timeout");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        return 5000;
    }
    return lua_tointeger(L, lua_gettop(L));
}

bool get_use_latency_aware_routing(lua_State *L, int i)
{
    lua_getfield(L, i, "use_latency_aware_routing");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        return false;
    }
    return lua_toboolean(L, lua_gettop(L));
}

const char *get_contact_points(lua_State *L, int i)
{
    lua_getfield(L, i, "contact_points");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        return "127.0.0.1";
    }
    return lua_tostring(L, lua_gettop(L));
}

const char *get_application_name(lua_State *L, int i)
{
    lua_getfield(L, i, "application_name");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        return NULL;
    }
    return lua_tostring(L, lua_gettop(L));
}

bool get_reconnect(lua_State *L, int i)
{
    lua_getfield(L, i, "reconnect");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        return false;
    }
    return lua_toboolean(L, lua_gettop(L));
}

static int connect(lua_State *L)
{
    lucas_log(LOG_INFO, "Attempting to connect");
    const int ARG_OPTIONS = 1;
    CassFuture *future = NULL;
    LucasError *rc = NULL;
    CassCluster *cluster = NULL;
    CassError err = CASS_OK;

    const char *contact_points = get_contact_points(L, ARG_OPTIONS);
    const int port = get_port(L, ARG_OPTIONS);
    const int num_threads_io = get_num_threads_io(L, ARG_OPTIONS);
    const char *application_name = get_application_name(L, ARG_OPTIONS);
    const bool use_latency_aware_routing = get_use_latency_aware_routing(L, ARG_OPTIONS);
    const bool reconnect = get_reconnect(L, ARG_OPTIONS);
    const int connect_timeout = get_connect_timeout(L, ARG_OPTIONS);

    if (!reconnect && session != NULL)
    {
        lucas_log(LOG_WARN, "already connected");
        return 0;
    }
    if (session != NULL)
    {
        lucas_log(LOG_WARN, "freeing existing session");
        cass_session_free(session);
    }
    session = cass_session_new();
    cluster = cass_cluster_new();
    err = cass_cluster_set_contact_points(cluster, contact_points);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set contact points %s", contact_points);
        goto cleanup;
    }
    err = cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V4);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set protocol version");
        goto cleanup;
    }
    err = cass_cluster_set_port(cluster, port);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set port %d", port);
        goto cleanup;
    }
    err = cass_cluster_set_num_threads_io(cluster, num_threads_io);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set IO thread count to %d", num_threads_io);
        goto cleanup;
    }
    cass_cluster_set_constant_reconnect(cluster, 1000);
    cass_cluster_set_connect_timeout(cluster, connect_timeout);
    cass_cluster_set_application_name(cluster, application_name);
    cass_cluster_set_latency_aware_routing(cluster, use_latency_aware_routing);
    lucas_log(LOG_INFO, "session configuration done, ready to connect");

    future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not connect to cluster");
        goto cleanup;
    }
    lucas_log(LOG_INFO, "session connect success");

cleanup:
    if (future)
    {
        cass_future_free(future);
    }
    if (cluster)
    {
        cass_cluster_free(cluster);
    }
    if (rc)
    {
        lucas_error_to_lua(L, rc);
    }
    return 0;
}

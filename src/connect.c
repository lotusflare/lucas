#include "cassandra.h"
#include "errors.c"
#include "logging.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include "types.c"

int get_port(lua_State *L, int i)
{
    lua_getfield(L, i, "port");
    int port = lua_tointeger(L, lua_gettop(L));
    if (port == 0)
    {
        return 9042;
    }
    return port;
}

int get_num_threads_io(lua_State *L, int i)
{
    lua_getfield(L, i, "num_threads_io");
    int num = lua_tointeger(L, lua_gettop(L));
    if (num == 0)
    {
        return 1;
    }
    return num;
}

bool get_use_latency_aware_routing(lua_State *L, int i)
{
    lua_getfield(L, i, "use_latency_aware_routing");
    if (lua_type(L, i) == LUA_TNIL)
    {
        return false;
    }
    return lua_toboolean(L, lua_gettop(L));
}

const char *get_contact_points(lua_State *L, int i)
{
    lua_getfield(L, i, "contact_points");
    return lua_tostring(L, lua_gettop(L));
}

const char *get_application_name(lua_State *L, int i)
{
    lua_getfield(L, i, "application_name");
    return lua_tostring(L, lua_gettop(L));
}

bool get_reconnect(lua_State *L, int i)
{
    lua_getfield(L, i, "reconnect");
    return lua_toboolean(L, lua_gettop(L));
}

static int connect(lua_State *L)
{
    lucas_log(CASS_LOG_INFO, "Attempting to connect");
    const int ARG_OPTIONS = 1;
    const char *contact_points = get_contact_points(L, ARG_OPTIONS);
    const int port = get_port(L, ARG_OPTIONS);
    const int num_threads_io = get_num_threads_io(L, ARG_OPTIONS);
    const char *application_name = get_application_name(L, ARG_OPTIONS);
    cass_bool_t use_latency_aware_routing = get_use_latency_aware_routing(L, ARG_OPTIONS);
    const bool reconnect = get_reconnect(L, ARG_OPTIONS);
    CassFuture *future = NULL;
    LucasError *rc = NULL;
    if (!reconnect && session != NULL)
    {
        lucas_log(CASS_LOG_INFO, "already connected");
        return 0;
    }
    if (session != NULL)
    {
        cass_session_free(session);
    }
    session = cass_session_new();
    CassCluster *cluster = cass_cluster_new();
    CassError err = cass_cluster_set_contact_points(cluster, contact_points);
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
        rc = lucas_new_errorf_from_cass_error(err, "could not set IO thread count");
        goto cleanup;
    }
    cass_cluster_set_constant_reconnect(cluster, 1000);
    cass_cluster_set_connect_timeout(cluster, 5000);
    cass_cluster_set_application_name(cluster, application_name);
    cass_cluster_set_latency_aware_routing(cluster, use_latency_aware_routing);
    future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not connect to %s", contact_points);
        goto cleanup;
    }

cleanup:
    cass_future_free(future);
    cass_cluster_free(cluster);
    if (rc)
    {
        lucas_error_to_lua(L, rc);
    }
    return 0;
}

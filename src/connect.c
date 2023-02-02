#include "cassandra.h"
#include "errors.c"
#include "logs.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include "types.c"

LucasError *set_port(lua_State *L, int i, CassCluster *cluster)
{
    LucasError *rc = NULL;
    int port;
    lua_getfield(L, i, "port");
    if (lua_type(L, lua_gettop(L)) == LUA_TNIL)
    {
        port = 9042;
    }
    else
    {
        port = lua_tointeger(L, lua_gettop(L));
    }
    CassError err = cass_cluster_set_port(cluster, port);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set port to %d", port);
    }
    return rc;
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

int get_connection_heartbeat_interval(lua_State *L, int i)
{
    return 1000;
}

void set_constant_reconnect(lua_State *L, int i, CassCluster *cluster)
{
    const int constant_reconnect = 1000;
    cass_cluster_set_constant_reconnect(cluster, constant_reconnect);
}

LucasError *set_protocol_version(lua_State *L, int i, CassCluster *cluster)
{
    LucasError *rc = NULL;
    const int protocol_version = CASS_PROTOCOL_VERSION_V4;
    CassError err = cass_cluster_set_protocol_version(cluster, protocol_version);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set protocol version to %d", protocol_version);
    }
    return rc;
}

LucasError *set_ssl(lua_State *L, int i, CassCluster *cluster)
{
    LucasError *rc = NULL;
    CassError err = CASS_OK;
    CassSsl *ssl = cass_ssl_new();
    lua_getfield(L, i, "ssl");

    lua_getfield(L, lua_gettop(L), "certificate");
    const char *cert = lua_tostring(L, lua_gettop(L));
    err = cass_ssl_set_cert(ssl, cert);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "failed to set certificate");
        goto cleanup;
    }

    lua_getfield(L, lua_gettop(L), "private_key");
    const char *private_key = lua_tostring(L, lua_gettop(L));
    err = cass_ssl_set_private_key(ssl, private_key, NULL);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "failed to set private key");
        goto cleanup;
    }

    cass_cluster_set_ssl(cluster, ssl);

cleanup:
    cass_ssl_free(ssl);
    return rc;
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
    const char *application_name = get_application_name(L, ARG_OPTIONS);
    const int num_threads_io = get_num_threads_io(L, ARG_OPTIONS);
    const int connect_timeout = get_connect_timeout(L, ARG_OPTIONS);
    const int connection_heartbeat_interval = get_connection_heartbeat_interval(L, ARG_OPTIONS);
    const bool use_latency_aware_routing = get_use_latency_aware_routing(L, ARG_OPTIONS);
    const bool reconnect = get_reconnect(L, ARG_OPTIONS);

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
    rc = set_protocol_version(L, ARG_OPTIONS, cluster);
    if (rc)
    {
        goto cleanup;
    }
    rc = set_port(L, ARG_OPTIONS, cluster);
    if (rc)
    {
        goto cleanup;
    }
    err = cass_cluster_set_num_threads_io(cluster, num_threads_io);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not set IO thread count to %d", num_threads_io);
        goto cleanup;
    }
    rc = set_ssl(L, ARG_OPTIONS, cluster);
    if (rc)
    {
        goto cleanup;
    }
    cass_cluster_set_connection_heartbeat_interval(cluster, connection_heartbeat_interval);
    set_constant_reconnect(L, ARG_OPTIONS, cluster);
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

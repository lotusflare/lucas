#include "cassandra.h"
#include "errors.c"
#include "logs.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include "types.c"
#include <string.h>

LucasError *set_port(lua_State *L, int i, CassCluster *cluster)
{
    int port = 9042;
    lua_getfield(L, i, "port");
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        port = lua_tointeger(L, lua_gettop(L));
    }
    CassError err = cass_cluster_set_port(cluster, port);
    if (err != CASS_OK)
    {
        return lucas_new_errorf_from_cass_error(err, "could not set port to %d", port);
    }
    return NULL;
}

LucasError *set_num_threads_io(lua_State *L, int i, CassCluster *cluster)
{
    int num_threads_io = 1;
    lua_getfield(L, i, "num_threads_io");
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        num_threads_io = lua_tointeger(L, lua_gettop(L));
    }
    CassError err = cass_cluster_set_num_threads_io(cluster, num_threads_io);
    if (err != CASS_OK)
    {
        return lucas_new_errorf_from_cass_error(err, "could not set IO thread count to %d", num_threads_io);
    }
    return NULL;
}

void set_connect_timeout(lua_State *L, int i, CassCluster *cluster)
{
    int connect_timeout = 5000;
    lua_getfield(L, i, "connect_timeout");
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        connect_timeout = lua_tointeger(L, lua_gettop(L));
    }
    cass_cluster_set_connect_timeout(cluster, connect_timeout);
}

void set_use_latency_aware_routing(lua_State *L, int i, CassCluster *cluster)
{
    int latency_aware_routing = false;
    lua_getfield(L, i, "use_latency_aware_routing");
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        latency_aware_routing = lua_toboolean(L, lua_gettop(L));
    }
    cass_cluster_set_latency_aware_routing(cluster, latency_aware_routing);
}

LucasError *set_contact_points(lua_State *L, int i, CassCluster *cluster)
{
    lua_getfield(L, i, "contact_points");
    const char *contact_points = "127.0.0.1";
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        contact_points = lua_tostring(L, lua_gettop(L));
    }
    CassError err = cass_cluster_set_contact_points(cluster, contact_points);
    if (err != CASS_OK)
    {
        return lucas_new_errorf_from_cass_error(err, "could not set contact points %s", contact_points);
    }
    return NULL;
}

void set_application_name(lua_State *L, int i, CassCluster *cluster)
{
    lua_getfield(L, i, "application_name");
    const char *application_name = NULL;
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        application_name = lua_tostring(L, lua_gettop(L));
    }
    cass_cluster_set_application_name(cluster, application_name);
}

void set_connection_heartbeat_interval(lua_State *L, int i, CassCluster *cluster)
{
    int heartbeat_interval = 1000;
    lua_getfield(L, i, "heartbeat_interval");
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        heartbeat_interval = lua_tointeger(L, lua_gettop(L));
    }
    cass_cluster_set_connection_heartbeat_interval(cluster, heartbeat_interval);
}

void set_constant_reconnect(lua_State *L, int i, CassCluster *cluster)
{
    int constant_reconnect = 1000;
    lua_getfield(L, i, "constant_reconnect");
    if (lua_type(L, lua_gettop(L)) != LUA_TNIL)
    {
        constant_reconnect = lua_tointeger(L, lua_gettop(L));
    }
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
    int ssl_index = lua_gettop(L);
    if (lua_type(L, ssl_index) == LUA_TNIL)
    {
        lucas_log(LOG_WARN, "ssl options not provided");
        return NULL;
    }
    lua_getfield(L, ssl_index, "certificate");
    const char *cert = lua_tostring(L, lua_gettop(L));
    lucas_log(LOG_DEBUG, "cert loaded, size=%d", strlen(cert));
    err = cass_ssl_set_cert(ssl, cert);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "failed to set certificate");
        goto cleanup;
    }
    lua_getfield(L, ssl_index, "private_key");
    const char *private_key = lua_tostring(L, lua_gettop(L));
    lucas_log(LOG_DEBUG, "key loaded, size=%d", strlen(private_key));
    err = cass_ssl_set_private_key(ssl, private_key, NULL);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "failed to set private key");
        goto cleanup;
    }
    cass_ssl_set_verify_flags(ssl, CASS_SSL_VERIFY_NONE);
    cass_cluster_set_ssl(cluster, ssl);
    lucas_log(LOG_INFO, "ssl configured");
cleanup:
    cass_ssl_free(ssl);
    return rc;
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
    lucas_log(LOG_INFO, "attempting to connect");
    const int ARG_OPTIONS = 1;
    CassFuture *future = NULL;
    LucasError *rc = NULL;
    CassCluster *cluster = NULL;
    CassError err = CASS_OK;
    const bool reconnect = get_reconnect(L, ARG_OPTIONS);

    if (session && !reconnect)
    {
        lucas_log(LOG_WARN, "already connected");
        return 0;
    }
    if (session)
    {
        lucas_log(LOG_WARN, "freeing existing session");
        cass_session_free(session);
    }
    session = cass_session_new();
    cluster = cass_cluster_new();
    rc = set_contact_points(L, ARG_OPTIONS, cluster);
    if (rc)
    {
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
    rc = set_num_threads_io(L, ARG_OPTIONS, cluster);
    if (rc)
    {
        goto cleanup;
    }
    rc = set_ssl(L, ARG_OPTIONS, cluster);
    if (rc)
    {
        goto cleanup;
    }
    set_use_latency_aware_routing(L, ARG_OPTIONS, cluster);
    set_connection_heartbeat_interval(L, ARG_OPTIONS, cluster);
    set_constant_reconnect(L, ARG_OPTIONS, cluster);
    set_connect_timeout(L, ARG_OPTIONS, cluster);
    set_application_name(L, ARG_OPTIONS, cluster);
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

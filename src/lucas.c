#include "cassandra.h"
#include "compat.c"
#include "errors.c"
#include "logging.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "querying.c"
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

const char *get_application_name(lua_State *L, int i)
{
    lua_getfield(L, i, "application_name");
    return lua_tostring(L, lua_gettop(L));
}

bool get_use_latency_aware_routing(lua_State *L, int i)
{
    lua_getfield(L, i, "use_latency_aware_routing");
    return lua_toboolean(L, lua_gettop(L));
}

bool get_reconnect(lua_State *L, int i)
{
    lua_getfield(L, i, "reconnect");
    return lua_toboolean(L, lua_gettop(L));
}

static int connect(lua_State *L)
{
    lucas_log(CASS_LOG_INFO, "Attempting to connect");
    const int ARG_CONTACT_POINTS = 1;
    const int ARG_OPTIONS = 2;
    luaL_checkstring(L, ARG_CONTACT_POINTS);
    const char *contact_points = lua_tostring(L, ARG_CONTACT_POINTS);
    const int port = get_port(L, ARG_OPTIONS);
    const int num_threads_io = get_num_threads_io(L, ARG_OPTIONS);
    const char *application_name = get_application_name(L, ARG_OPTIONS);
    cass_bool_t use_latency_aware_routing = get_use_latency_aware_routing(L, ARG_OPTIONS);
    const bool reconnect = get_reconnect(L, ARG_OPTIONS);
    LucasError *rc = NULL;

    if (!reconnect && session != NULL)
    {
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
    cass_cluster_set_connect_timeout(cluster, 5000);
    cass_cluster_set_application_name(cluster, application_name);
    cass_cluster_set_latency_aware_routing(cluster, use_latency_aware_routing);
    CassFuture *future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "could not connect");
        goto cleanup;
    }
cleanup:
    if (future)
    {
        cass_future_free(future);
    }
    if (cluster)
    {
        cass_cluster_free(cluster);
    }
    if (rc != NULL)
    {
        lucas_error_to_lua(L, rc);
    }
    return 0;
}

int luaopen_lucas(lua_State *L)
{
    luaL_Reg reg[] = {
        {"connect", connect},
        {"query", query},
        {"batch", batch},
        {"logger", logger},
        {"metrics", metrics},
        {"speculative_execution_metrics", speculative_execution_metrics},

        {"ascii", type_ascii},
        {"bigint", type_bigint},
        {"blob", type_blob},
        {"boolean", type_boolean},
        {"counter", type_counter},
        {"decimal", type_decimal},
        {"double", type_double},
        {"float", type_float},
        {"int", type_int},
        {"text", type_text},
        {"timestamp", type_timestamp},
        {"uuid", type_uuid},
        {"varchar", type_varchar},
        {"varint", type_varint},
        {"timeuuid", type_timeuuid},
        {"inet", type_inet},
        {"date", type_date},
        {"time", type_time},
        {"smallint", type_smallint},
        {"tinyint", type_tinyint},
        {"duration", type_duration},
        {"list", type_list},
        {"map", type_map},
        {"set", type_set},
        {"udt", type_udt},
        {"tuple", type_tuple},
        {"null", type_null},
        {"unset", type_unset},

        {NULL, NULL},
    };
    luaL_Reg reg_compat[] = {
        {"convert", convert},

        {NULL, NULL},
    };
    luaL_openlib(L, "lucas", reg, 0);
    luaL_openlib(L, "lucas.compatibility", reg_compat, 0);
    cass_log_set_level(CASS_LOG_DISABLED);
    return 2;
}

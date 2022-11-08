#include "cassandra.h"
#include "errors.c"
#include "logging.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "querying.c"
#include "state.c"
#include "types.c"

static int connect(lua_State *L)
{
    lucas_log(CASS_LOG_INFO, "Attempting to connect");
    const int ARG_CONTACT_POINTS = 1;
    const int ARG_PORT = 2;
    luaL_checkstring(L, ARG_CONTACT_POINTS);
    const char *contact_points = lua_tostring(L, ARG_CONTACT_POINTS);
    int port = lua_tointeger(L, ARG_PORT);
    if (port == 0)
    {
        port = 9042;
    }
    session = cass_session_new();
    cluster = cass_cluster_new();
    CassError err = cass_cluster_set_contact_points(cluster, contact_points);
    if (err != CASS_OK)
    {
        errorf_cass_to_lua(L, err, "could not set contact points %s", contact_points);
    }
    err = cass_cluster_set_protocol_version(cluster, CASS_PROTOCOL_VERSION_V4);
    if (err != CASS_OK)
    {
        errorf_cass_to_lua(L, err, "could not set protocol version");
    }
    err = cass_cluster_set_port(cluster, port);
    if (err != CASS_OK)
    {
        errorf_cass_to_lua(L, err, "could not set port %d", port);
    }
    CassFuture *future = cass_session_connect(session, cluster);
    cass_future_wait(future);
    err = cass_future_error_code(future);
    cass_future_free(future);
    if (err != CASS_OK)
    {
        errorf_cass_to_lua(L, err, "could not connect");
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
    luaL_openlib(L, "lucas", reg, 0);
    cass_log_set_level(CASS_LOG_DISABLED);
    return 1;
}

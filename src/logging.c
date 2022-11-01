#include "cassandra.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"

typedef struct Metric
{
    const char *key;
    lua_Number value;
} Metric;

void push_metrics(lua_State *L, int index, Metric items[], int length)
{
    for (int i = 0; i < length; i++)
    {
        lua_pushstring(L, items[i].key);
        lua_pushnumber(L, items[i].value);
        lua_settable(L, index);
    }
}

int speculative_execution_metrics(lua_State *L)
{
    CassSpeculativeExecutionMetrics metrics;
    cass_session_get_speculative_execution_metrics(session, &metrics);

    lua_newtable(L);
    int table = lua_gettop(L);
    Metric items[] = {
        {"count", metrics.count},
        {"max", metrics.max},
        {"mean", metrics.mean},
        {"median", metrics.median},
        {"min", metrics.min},
        {"percentage", metrics.percentage},
        {"percentile_75th", metrics.percentile_75th},
        {"percentile_95th", metrics.percentile_95th},
        {"percentile_98th", metrics.percentile_98th},
        {"percentile_99th", metrics.percentile_99th},
        {"percentile_999th", metrics.percentile_999th},
    };
    push_metrics(L, table, items, sizeof(items) / sizeof(Metric));

    return 1;
}

int metrics(lua_State *L)
{
    CassMetrics metrics;
    cass_session_get_metrics(session, &metrics);
    lua_newtable(L);
    int main_table = lua_gettop(L);

    // stats subtable
    lua_pushstring(L, "stats");
    lua_newtable(L);
    int stats_table = lua_gettop(L);
    Metric stats_items[] = {
        {"total_connections", metrics.stats.total_connections},
        {"available_connections", metrics.stats.available_connections},
        {"exceeded_pending_requests_water_mark", metrics.stats.exceeded_pending_requests_water_mark},
        {"exceeded_write_bytes_water_mark", metrics.stats.exceeded_write_bytes_water_mark},
    };
    push_metrics(L, stats_table, stats_items, sizeof(stats_items) / sizeof(Metric));
    lua_settable(L, main_table);

    // requests subtable
    lua_pushstring(L, "requests");
    lua_newtable(L);
    int requests_table = lua_gettop(L);
    Metric requests_items[] = {
        {"stddev", metrics.requests.stddev},
        {"fifteen_minute_rate", metrics.requests.fifteen_minute_rate},
        {"five_minute_rate", metrics.requests.five_minute_rate},
        {"one_minute_rate", metrics.requests.one_minute_rate},
        {"mean_rate", metrics.requests.mean_rate},
        {"max", metrics.requests.max},
        {"mean", metrics.requests.mean},
        {"mean_rate", metrics.requests.mean_rate},
        {"min", metrics.requests.min},
        {"median", metrics.requests.median},
        {"percentile_75th", metrics.requests.percentile_75th},
        {"percentile_95th", metrics.requests.percentile_95th},
        {"percentile_98th", metrics.requests.percentile_98th},
        {"percentile_999th", metrics.requests.percentile_999th},
        {"percentile_99th", metrics.requests.percentile_99th},
    };
    push_metrics(L, requests_table, requests_items, sizeof(requests_items) / sizeof(Metric));
    lua_settable(L, main_table);

    // errors subtable
    lua_pushstring(L, "errors");
    lua_newtable(L);
    int errors_table = lua_gettop(L);
    Metric errors_items[] = {
        {"connection_timeouts", metrics.errors.connection_timeouts},
        {"pending_request_timeouts", metrics.errors.pending_request_timeouts},
        {"request_timeouts", metrics.errors.request_timeouts},
    };
    push_metrics(L, errors_table, errors_items, sizeof(errors_items) / sizeof(Metric));
    lua_settable(L, main_table);

    return 1;
}

void callback(const CassLogMessage *log, void *data)
{
    lua_pushvalue(log_context, 1);
    lua_pushstring(log_context, log->message);
    lua_pushinteger(log_context, log->severity);
    lua_call(log_context, 2, 0);
}

int logger(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    log_context = lua_newthread(L);
    luaL_ref(L, LUA_REGISTRYINDEX);
    lua_xmove(L, log_context, 1);
    cass_log_set_callback(callback, NULL);
    cass_log_set_level(CASS_LOG_DEBUG);
    return 0;
}

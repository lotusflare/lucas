
#include "cassandra.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include <pthread.h>
#include <stdarg.h>
#include <time.h>

typedef struct Metric
{
    const char *key;
    lua_Number value;
} Metric;

void push_metrics(lua_State *L, int index, Metric items[], unsigned long length)
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
    int spec_table = lua_gettop(L);
    Metric spec_metrics[] = {
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
    push_metrics(L, spec_table, spec_metrics, sizeof(spec_metrics) / sizeof(Metric));

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
    Metric stats_metrics[] = {
        {"total_connections", metrics.stats.total_connections},
        {"available_connections", metrics.stats.available_connections},
        {"exceeded_pending_requests_water_mark", metrics.stats.exceeded_pending_requests_water_mark},
        {"exceeded_write_bytes_water_mark", metrics.stats.exceeded_write_bytes_water_mark},
    };
    push_metrics(L, stats_table, stats_metrics, sizeof(stats_metrics) / sizeof(Metric));
    lua_settable(L, main_table);

    // requests subtable
    lua_pushstring(L, "requests");
    lua_newtable(L);
    int requests_table = lua_gettop(L);
    Metric requests_metrics[] = {
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
    push_metrics(L, requests_table, requests_metrics, sizeof(requests_metrics) / sizeof(Metric));
    lua_settable(L, main_table);

    // errors subtable
    lua_pushstring(L, "errors");
    lua_newtable(L);
    int errors_table = lua_gettop(L);
    Metric errors_metrics[] = {
        {"connection_timeouts", metrics.errors.connection_timeouts},
        {"pending_request_timeouts", metrics.errors.pending_request_timeouts},
        {"request_timeouts", metrics.errors.request_timeouts},
    };
    push_metrics(L, errors_table, errors_metrics, sizeof(errors_metrics) / sizeof(Metric));
    lua_settable(L, main_table);

    return 1;
}
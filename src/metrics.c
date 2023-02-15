///
/// @file metrics.c
/// @brief Expose internal driver metrics.
/// @author Erik Berkun-Drevnig<erik.berkundrevnig@lotusflare.com>
/// @copyright Copyright (c) 2023 LotusFlare, Inc.
///

#include "cassandra.h"
#include "logs.c"
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

void push_metrics(lua_State *L, Metric items[], unsigned long length)
{
    lua_newtable(L);
    const int table = lua_gettop(L);

    for (int i = 0; i < length; i++)
    {
        lua_pushstring(L, items[i].key);
        lua_pushnumber(L, items[i].value);
        lua_settable(L, table);
    }
}

int speculative_execution_metrics(lua_State *L)
{
    CassSpeculativeExecutionMetrics metrics;
    cass_session_get_speculative_execution_metrics(session, &metrics);
    lucas_log(LOG_INFO, "preparing speculative execution metrics");

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
    push_metrics(L, spec_metrics, sizeof(spec_metrics) / sizeof(Metric));

    return 1;
}

int metrics(lua_State *L)
{
    CassMetrics metrics;
    cass_session_get_metrics(session, &metrics);
    lua_newtable(L);
    const int parent_table = lua_gettop(L);
    lucas_log(LOG_INFO, "preparing metrics");

    Metric stats_metrics[] = {
        {"total_connections", metrics.stats.total_connections},
        {"available_connections", metrics.stats.available_connections},
        {"exceeded_pending_requests_water_mark", metrics.stats.exceeded_pending_requests_water_mark},
        {"exceeded_write_bytes_water_mark", metrics.stats.exceeded_write_bytes_water_mark},
    };
    lua_pushstring(L, "stats");
    push_metrics(L, stats_metrics, sizeof(stats_metrics) / sizeof(Metric));
    lua_settable(L, parent_table);

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
    lua_pushstring(L, "requests");
    push_metrics(L, requests_metrics, sizeof(requests_metrics) / sizeof(Metric));
    lua_settable(L, parent_table);

    Metric errors_metrics[] = {
        {"connection_timeouts", metrics.errors.connection_timeouts},
        {"pending_request_timeouts", metrics.errors.pending_request_timeouts},
        {"request_timeouts", metrics.errors.request_timeouts},
    };
    lua_pushstring(L, "errors");
    push_metrics(L, errors_metrics, sizeof(errors_metrics) / sizeof(Metric));
    lua_settable(L, parent_table);

    return 1;
}

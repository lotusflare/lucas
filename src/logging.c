#include "cassandra.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"

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

int speculative_execution_metrics(lua_State *L)
{
    CassSpeculativeExecutionMetrics metrics;
    cass_session_get_speculative_execution_metrics(session, &metrics);
    lua_newtable(L);
    int table = lua_gettop(L);

    lua_pushstring(L, "count");
    lua_pushnumber(L, metrics.count);
    lua_settable(L, table);
    lua_pushstring(L, "max");
    lua_pushnumber(L, metrics.max);
    lua_settable(L, table);
    lua_pushstring(L, "mean");
    lua_pushnumber(L, metrics.mean);
    lua_settable(L, table);
    lua_pushstring(L, "median");
    lua_pushnumber(L, metrics.median);
    lua_settable(L, table);
    lua_pushstring(L, "min");
    lua_pushnumber(L, metrics.min);
    lua_settable(L, table);
    lua_pushstring(L, "percentage");
    lua_pushnumber(L, metrics.percentage);
    lua_settable(L, table);
    lua_pushstring(L, "percentile_75th");
    lua_pushnumber(L, metrics.percentile_75th);
    lua_settable(L, table);
    lua_pushstring(L, "percentile_95th");
    lua_pushnumber(L, metrics.percentile_95th);
    lua_settable(L, table);
    lua_pushstring(L, "percentile_98th");
    lua_pushnumber(L, metrics.percentile_98th);
    lua_settable(L, table);
    lua_pushstring(L, "percentile_99th");
    lua_pushnumber(L, metrics.percentile_99th);
    lua_settable(L, table);
    lua_pushstring(L, "percentile_999th");
    lua_pushnumber(L, metrics.percentile_999th);
    lua_settable(L, table);

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
    lua_pushstring(L, "total_connections");
    lua_pushnumber(L, metrics.stats.total_connections);
    lua_settable(L, stats_table);
    lua_pushstring(L, "available_connections");
    lua_pushnumber(L, metrics.stats.available_connections);
    lua_settable(L, stats_table);
    lua_pushstring(L, "exceeded_pending_requests_water_mark");
    lua_pushnumber(L, metrics.stats.exceeded_pending_requests_water_mark);
    lua_settable(L, stats_table);
    lua_pushstring(L, "exceeded_write_bytes_water_mark");
    lua_pushnumber(L, metrics.stats.exceeded_write_bytes_water_mark);
    lua_settable(L, stats_table);
    lua_settable(L, main_table);

    // requests subtable
    lua_pushstring(L, "requests");
    lua_newtable(L);
    int requests_table = lua_gettop(L);
    lua_pushstring(L, "stddev");
    lua_pushnumber(L, metrics.requests.stddev);
    lua_settable(L, requests_table);
    lua_pushstring(L, "fifteen_minute_rate");
    lua_pushnumber(L, metrics.requests.fifteen_minute_rate);
    lua_settable(L, requests_table);
    lua_pushstring(L, "five_minute_rate");
    lua_pushnumber(L, metrics.requests.five_minute_rate);
    lua_settable(L, requests_table);
    lua_pushstring(L, "one_minute_rate");
    lua_pushnumber(L, metrics.requests.one_minute_rate);
    lua_settable(L, requests_table);
    lua_pushstring(L, "mean_rate");
    lua_pushnumber(L, metrics.requests.mean_rate);
    lua_settable(L, requests_table);
    lua_pushstring(L, "max");
    lua_pushnumber(L, metrics.requests.max);
    lua_settable(L, requests_table);
    lua_pushstring(L, "mean");
    lua_pushnumber(L, metrics.requests.mean);
    lua_settable(L, requests_table);
    lua_pushstring(L, "mean_rate");
    lua_pushnumber(L, metrics.requests.mean_rate);
    lua_settable(L, requests_table);
    lua_pushstring(L, "min");
    lua_pushnumber(L, metrics.requests.min);
    lua_settable(L, requests_table);
    lua_pushstring(L, "median");
    lua_pushnumber(L, metrics.requests.median);
    lua_settable(L, requests_table);
    lua_pushstring(L, "percentile_75th");
    lua_pushnumber(L, metrics.requests.percentile_75th);
    lua_settable(L, requests_table);
    lua_pushstring(L, "percentile_95th");
    lua_pushnumber(L, metrics.requests.percentile_95th);
    lua_settable(L, requests_table);
    lua_pushstring(L, "percentile_98th");
    lua_pushnumber(L, metrics.requests.percentile_98th);
    lua_settable(L, requests_table);
    lua_pushstring(L, "percentile_999th");
    lua_pushnumber(L, metrics.requests.percentile_999th);
    lua_settable(L, requests_table);
    lua_pushstring(L, "percentile_99th");
    lua_pushnumber(L, metrics.requests.percentile_99th);
    lua_settable(L, requests_table);
    lua_settable(L, main_table);

    // errors subtable
    lua_pushstring(L, "errors");
    lua_newtable(L);
    int errors_table = lua_gettop(L);
    lua_pushstring(L, "connection_timeouts");
    lua_pushnumber(L, metrics.errors.connection_timeouts);
    lua_settable(L, errors_table);
    lua_pushstring(L, "pending_request_timeouts");
    lua_pushnumber(L, metrics.errors.pending_request_timeouts);
    lua_settable(L, errors_table);
    lua_pushstring(L, "request_timeouts");
    lua_pushnumber(L, metrics.errors.request_timeouts);
    lua_settable(L, errors_table);
    lua_settable(L, main_table);

    return 1;
}

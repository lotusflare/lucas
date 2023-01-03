#pragma once

#include "cassandra.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

typedef enum LucasLogLevel
{
    LucasLogCritical = 1,
    LucasLogError = 2,
    LucasLogWarn = 3,
    LucasLogInfo = 4,
    LucasLogDebug = 5,
    LucasLogTrace = 6,
} LucasLogLevel;

LucasLogLevel log_visibility = LucasLogInfo;

void log_lua(const char *message, LucasLogLevel severity, int timestamp)
{
    if (severity > log_visibility)
    {
        return;
    }

    pthread_mutex_lock(&lock);
    lua_pushvalue(log_context, 1);
    lua_pushstring(log_context, message);
    lua_pushinteger(log_context, severity);
    lua_pushinteger(log_context, timestamp);
    int result = lua_pcall(log_context, 3, 0, 0); // results == LUA_OK
    pthread_mutex_unlock(&lock);
}

void lucas_log(LucasLogLevel level, const char *fmt, ...)
{
    if (!log_context)
    {
        return;
    }

    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    char append[vsnprintf(NULL, 0, fmt, args1) + 1];
    vsprintf(append, fmt, args2);
    log_lua(append, level, (int)time(NULL));

    va_end(args1);
    va_end(args2);
}

LucasLogLevel lucas_log_level_from_cass(CassLogLevel cass_level)
{
    switch (cass_level)
    {
    case CASS_LOG_TRACE:
        return LucasLogTrace;
    case CASS_LOG_DEBUG:
        return LucasLogDebug;
    default:
        lucas_log(LucasLogWarn, "invalid CassLogLevel log level %d", cass_level);
    case CASS_LOG_INFO:
        return LucasLogInfo;
    case CASS_LOG_WARN:
        return LucasLogWarn;
    case CASS_LOG_ERROR:
        return LucasLogError;
    case CASS_LOG_CRITICAL:
        return LucasLogCritical;
    }
}

CassLogLevel cass_log_level_from_lucas(LucasLogLevel cass_level)
{
    switch (cass_level)
    {
    case LucasLogTrace:
        return CASS_LOG_TRACE;
    case LucasLogDebug:
        return CASS_LOG_DEBUG;
    default:
        lucas_log(LucasLogWarn, "invalid LucasLogLevel log level %d", cass_level);
    case LucasLogInfo:
        return CASS_LOG_INFO;
    case LucasLogWarn:
        return CASS_LOG_WARN;
    case LucasLogError:
        return CASS_LOG_ERROR;
    case LucasLogCritical:
        return CASS_LOG_CRITICAL;
    }
}

void cassandra_callback(const CassLogMessage *log, void *data)
{
    LucasLogLevel level = lucas_log_level_from_cass(log->severity);
    log_lua(log->message, level, log->time_ms / 1000);
}

int logger(lua_State *L)
{
    const int arg_count = lua_gettop(L);
    const int ARG_CALLBACK = 1;
    const int ARG_LEVEL = 2;

    if (arg_count > 1)
    {
        luaL_checktype(L, ARG_LEVEL, LUA_TNUMBER);
        log_visibility = lua_tointeger(L, ARG_LEVEL);
    }

    if (log_context)
    {
        lua_close(log_context);
    }

    luaL_checktype(L, ARG_CALLBACK, LUA_TFUNCTION);
    log_context = lua_newthread(L);
    luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, ARG_CALLBACK);
    lua_xmove(L, log_context, 1);
    cass_log_set_callback(cassandra_callback, NULL);
    cass_log_set_level(cass_log_level_from_lucas(log_visibility));
    return 0;
}

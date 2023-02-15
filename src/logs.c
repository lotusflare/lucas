///
/// @file logs.c
/// @brief Support for lucas and libcassandra logs.
/// @author Erik Berkun-Drevnig<eberkundrevnig@lotusflare.com>
/// @copyright Copyright (c) 2023 LotusFlare, Inc.
///

#pragma once

#include "cassandra.h"
#include "errors.c"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define LOG_SOURCE_MAPPING(XX)                                                                                         \
    XX(SOURCE_LUCAS, "lucas")                                                                                          \
    XX(SOURCE_CASSANDRA, "cassandra")

typedef enum LogSource
{
#define XX_LOG_SOURCE(source, _) source,
    LOG_SOURCE_MAPPING(XX_LOG_SOURCE)
#undef XX_LOG_SOURCE
} LogSource;

static const char *const LogSourceNames[] = {
#define XX_SOURCE_NAME(_, name) name,
    LOG_SOURCE_MAPPING(XX_SOURCE_NAME)
#undef XX_SOURCE_NAME
};

typedef enum LucasLogLevel
{
    LOG_CRITICAL = 1,
    LOG_ERROR = 2,
    LOG_WARN = 3,
    LOG_INFO = 4,
    LOG_DEBUG = 5,
    LOG_TRACE = 6,
} LucasLogLevel;

LucasLogLevel log_visibility = LOG_INFO;

void log_lua(const char *message, const char *source, LucasLogLevel severity, int timestamp)
{
    if (severity > log_visibility)
    {
        return;
    }

    int initial = lua_gettop(log_context);
    pthread_mutex_lock(&lock);
    lua_pushvalue(log_context, 1);
    lua_pushstring(log_context, message);
    lua_pushstring(log_context, source);
    lua_pushinteger(log_context, severity);
    lua_pushinteger(log_context, timestamp);
    if (lua_pcall(log_context, 4, 0, 0) != LUA_OK)
    {
        printf("error: %s\n", lua_tostring(log_context, -1));
    }
    lua_settop(log_context, initial);
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
    log_lua(append, LogSourceNames[SOURCE_LUCAS], level, (int)time(NULL));

    va_end(args1);
    va_end(args2);
}

LucasLogLevel lucas_log_level_from_cass(CassLogLevel cass_level)
{
    switch (cass_level)
    {
    case CASS_LOG_TRACE:
        return LOG_TRACE;
    case CASS_LOG_DEBUG:
        return LOG_DEBUG;
    default:
        lucas_log(LOG_WARN, "invalid CassLogLevel log level %d", cass_level);
    case CASS_LOG_INFO:
        return LOG_INFO;
    case CASS_LOG_WARN:
        return LOG_WARN;
    case CASS_LOG_ERROR:
        return LOG_ERROR;
    case CASS_LOG_CRITICAL:
        return LOG_CRITICAL;
    }
}

CassLogLevel cass_log_level_from_lucas(LucasLogLevel lucas_level)
{
    switch (lucas_level)
    {
    case LOG_TRACE:
        return CASS_LOG_TRACE;
    case LOG_DEBUG:
        return CASS_LOG_DEBUG;
    default:
        lucas_log(LOG_WARN, "invalid LucasLogLevel log level %d", lucas_level);
    case LOG_INFO:
        return CASS_LOG_INFO;
    case LOG_WARN:
        return CASS_LOG_WARN;
    case LOG_ERROR:
        return CASS_LOG_ERROR;
    case LOG_CRITICAL:
        return CASS_LOG_CRITICAL;
    }
}

void cassandra_callback(const CassLogMessage *log, void *data)
{
    LucasLogLevel level = lucas_log_level_from_cass(log->severity);
    log_lua(log->message, LogSourceNames[SOURCE_CASSANDRA], level, log->time_ms / 1000);
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
        lucas_log(LOG_WARN, "closing existing log context");
        lua_close(log_context);
    }

    luaL_checktype(L, ARG_CALLBACK, LUA_TFUNCTION);
    log_context = lua_newthread(L);
    luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, ARG_CALLBACK);
    lua_xmove(L, log_context, lua_gettop(L));
    cass_log_set_callback(cassandra_callback, NULL);
    cass_log_set_level(cass_log_level_from_lucas(log_visibility));
    return 0;
}

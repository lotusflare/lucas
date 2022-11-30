#pragma once

#include "cassandra.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"
#include "state.c"
#include <pthread.h>
#include <stdarg.h>
#include <time.h>

typedef CassLogLevel LucasLogLevel;

void log_lua(const char *message, LucasLogLevel severity, int timestamp)
{
    pthread_mutex_lock(&lock);
    lua_pushvalue(log_context, 1);
    lua_pushstring(log_context, message);
    lua_pushinteger(log_context, severity);
    lua_pushinteger(log_context, timestamp);
    lua_pcall(log_context, 3, 0, 0);
    pthread_mutex_unlock(&lock);
}

void cassandra_callback(const CassLogMessage *log, void *data)
{
    log_lua(log->message, log->severity, log->time_ms / 1000);
}

void lucas_log(LucasLogLevel level, const char *fmt, ...)
{
    if (log_context == NULL)
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

int logger(lua_State *L)
{
    if (log_context != NULL)
    {
        lua_close(log_context);
    }
    luaL_checktype(L, 1, LUA_TFUNCTION);
    log_context = lua_newthread(L);
    luaL_ref(L, LUA_REGISTRYINDEX);
    lua_xmove(L, log_context, 1);
    cass_log_set_callback(cassandra_callback, NULL);
    cass_log_set_level(CASS_LOG_DEBUG);
    return 0;
}

#pragma once

#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <stdarg.h>
#include <stdio.h>

const int MAX_LENGTH = 4096;

void error_to_lua(lua_State *L, const char *msg)
{
    lua_pushfstring(L, "error: %s\n", msg);
    lua_error(L);
}

void errorf_to_lua(lua_State *L, const char *fmt, ...)
{
    char msg[MAX_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    error_to_lua(L, msg);
}

void errorf_cass_to_lua(lua_State *L, CassError err, const char *fmt, ...)
{
    char append[MAX_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsprintf(append, fmt, args);
    va_end(args);
    const char *desc = cass_error_desc(err);
    errorf_to_lua(L, "%s: %s", append, desc);
}

void errorf_cass_future_to_lua(lua_State *L, CassFuture *future, const char *fmt, ...)
{
    size_t length;
    const char *desc;
    char append[MAX_LENGTH];
    cass_future_error_message(future, &desc, &length);
    sprintf(append, "%s: %s", desc, fmt);
    append[length] = 0; // cass_future_error_message may not return null terminated string
    va_list args;
    va_start(args, fmt);
    errorf_to_lua(L, append, args);
    va_end(args);
}

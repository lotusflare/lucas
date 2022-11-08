#pragma once

#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <stdarg.h>
#include <stdio.h>

void errorf_to_lua(lua_State *L, const char *fmt, ...)
{
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    char append[vsnprintf(NULL, 0, fmt, args1)];
    vsprintf(append, fmt, args2);

    lua_pushfstring(L, "lucas: %s", append);
    lua_error(L);

    va_end(args1);
    va_end(args2);
}

void errorf_cass_to_lua(lua_State *L, CassError err, const char *fmt, ...)
{
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    char append[snprintf(NULL, 0, fmt, args1)];
    vsprintf(append, fmt, args2);

    const char *desc = cass_error_desc(err);
    errorf_to_lua(L, "%s: %s", append, desc);

    va_end(args1);
    va_end(args2);
}

void errorf_cass_future_to_lua(lua_State *L, CassFuture *future, const char *fmt, ...)
{
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    char append[snprintf(NULL, 0, fmt, args1)];
    vsprintf(append, fmt, args2);

    size_t length;
    const char *desc;
    cass_future_error_message(future, &desc, &length);
    errorf_to_lua(L, "%s: %s", append, desc);

    va_end(args1);
    va_end(args2);
}

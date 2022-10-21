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
    va_list args;
    va_start(args, fmt);
    char msg[MAX_LENGTH];
    vsprintf(msg, fmt, args);
    error_to_lua(L, msg);
    va_end(args);
}

void errorf_cass_to_lua(lua_State *L, CassError err, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char msg[MAX_LENGTH];
    const char *desc = cass_error_desc(err);
    sprintf(msg, "%s: %s", desc, fmt);
    errorf_to_lua(L, msg, args);
    va_end(args);
}

void errorf_cass_future_to_lua(lua_State *L, CassFuture *future, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t length;
    const char *desc[MAX_LENGTH];
    char msg[MAX_LENGTH];
    cass_future_error_message(future, desc, &length);
    sprintf(msg, "%s: %s", *desc, fmt);
    errorf_to_lua(L, msg, args);
    va_end(args);
}

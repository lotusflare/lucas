#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <stdio.h>

void error_to_lua(lua_State *L, const char *fmt, ...)
{
    va_list args;
    char msg[1000];
    sprintf(msg, fmt, args);
    lua_pushstring(L, msg);
    lua_error(L);
}

void error_cass_to_lua(lua_State *L, CassError err, const char *fmt, ...)
{
    va_list args;
    char msg[1000];
    error_to_lua(L, "%s: %s", cass_error_desc(err), args);
}

void error_cass_future_to_lua(lua_State *L, CassFuture *future, const char *fmt, ...)
{
    va_list args;
    const char *future_message[1000];
    size_t message_length;
    cass_future_error_message(future, future_message, &message_length);
    error_to_lua(L, "%s: %s", *future_message, args);
}

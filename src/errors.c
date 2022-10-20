#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <stdio.h>

const int MAX_ERROR_MESSAGE_LENGTH = 4096;

void error_to_lua(lua_State *L, const char *fmt, ...)
{
    va_list args;
    char msg[MAX_ERROR_MESSAGE_LENGTH];
    printf(fmt, args);
    sprintf(msg, fmt, args);
    lua_pushstring(L, msg);
    lua_error(L);
}

void error_cass_to_lua(lua_State *L, CassError err, const char *fmt, ...)
{
    va_list args;
    char msg[MAX_ERROR_MESSAGE_LENGTH];
    error_to_lua(L, "%s: %s", cass_error_desc(err), args);
}

void error_cass_future_to_lua(lua_State *L, CassFuture *future, const char *fmt, ...)
{
    va_list args;
    size_t length;
    const char *message[MAX_ERROR_MESSAGE_LENGTH];
    cass_future_error_message(future, message, &length);
    error_to_lua(L, "%s: %s", *message, args);
}

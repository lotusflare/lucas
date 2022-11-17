#pragma once

#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct LucasError
{
    bool ok;
    char *message;
} LucasError;

void lucas_error_free(LucasError *err)
{
    free(err);
}

void lucas_error_to_lua(lua_State *L, LucasError *err)
{
    lua_pushfstring(L, "lucas: %s", err->message);
    lua_error(L);
}

LucasError *lucas_new_errorf(const char *fmt, ...)
{
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    LucasError *err = malloc(sizeof(LucasError));
    err->message = malloc(vsnprintf(NULL, 0, fmt, args1));
    vsprintf(err->message, fmt, args2);

    va_end(args1);
    va_end(args2);
    return err;
}

LucasError *lucas_new_errorf_from_cass_error(CassError cass_error, const char *fmt, ...)
{
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    char append[snprintf(NULL, 0, fmt, args1)];
    vsprintf(append, fmt, args2);

    const char *desc = cass_error_desc(cass_error);
    LucasError *err = lucas_new_errorf("%s: %s", append, desc);

    va_end(args1);
    va_end(args2);
    return err;
}

LucasError *lucas_new_errorf_from_cass_future(CassFuture *future, const char *fmt, ...)
{
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    char append[snprintf(NULL, 0, fmt, args1)];
    vsprintf(append, fmt, args2);

    size_t length;
    const char *desc;
    cass_future_error_message(future, &desc, &length);
    LucasError *err = lucas_new_errorf("%s: %s", append, desc);

    va_end(args1);
    va_end(args2);
    return err;
}

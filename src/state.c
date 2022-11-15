#pragma once

#include "cassandra.h"
#include <luajit-2.1/lua.h>

CassSession *session = NULL;
lua_State *log_context = NULL;

#pragma once

#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <pthread.h>

CassSession *session = NULL;
lua_State *log_context = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

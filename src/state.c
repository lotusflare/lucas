///
/// @file state.c
/// @brief Initialize driver state.
/// @author Erik Berkun-Drevnig<erik.berkundrevnig@lotusflare.com>
/// @copyright Copyright (c) 2023 LotusFlare, Inc.
///

#pragma once

#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <pthread.h>

CassSession *session = NULL;
lua_State *log_context = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

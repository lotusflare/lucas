///
/// @file query.h
/// @brief Definitions for query.c
/// @author Erik Berkun-Drevnig<erik.berkundrevnig@lotusflare.com>
/// @copyright Copyright (c) 2023 LotusFlare, Inc.
///

#include "errors.h"
#include <cassandra.h>
#include <luajit-2.1/lua.h>

LucasError *create_map(lua_State *L, int index, CassCollection **collection);
LucasError *append_collection(lua_State *L, int index, CassCollection *collection);
LucasError *create_collection(lua_State *L, int index, CassCollectionType type, CassCollection **collection);

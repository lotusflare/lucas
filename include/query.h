#include "errors.h"
#include <cassandra.h>
#include <luajit-2.1/lua.h>

LucasError *create_map(lua_State *L, int index, CassCollection **collection);
LucasError *append_collection(lua_State *L, int index, CassCollection *collection);

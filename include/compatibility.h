#include "errors.h"
#include <cassandra.h>
#include <luajit-2.1/lua.h>

LucasError *convert_table(lua_State *L, int index, int return_table);

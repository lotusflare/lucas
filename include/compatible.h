#include "errors.h"
#include <cassandra.h>
#include <luajit-2.1/lua.h>

LucasError *handle(lua_State *L, int value_index);
LucasError *handle_collection(lua_State *L, int index, int table, CassValueType *cvt_override);
LucasError *handle_table(lua_State *L, int index, int return_table);

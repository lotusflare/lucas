#include "cassandra.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/lua.h"

static lua_State *log_context = NULL;

void callback(const CassLogMessage *log, void *data)
{
    lua_pushvalue(log_context, 1);
    lua_pushstring(log_context, log->message);
    lua_pushinteger(log_context, log->severity);
    lua_call(log_context, 2, 0);
}

int log_callback(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    log_context = lua_newthread(L);
    luaL_ref(L, LUA_REGISTRYINDEX);
    lua_xmove(L, log_context, 1);
    cass_log_set_callback(callback, NULL);
    cass_log_set_level(CASS_LOG_DEBUG);
    return 0;
}

#include "library.h"
#include <iostream>
#include "luajit-2.1/lua.h"
#include "cassandra.h"

CassSession *session;

void connect() {
    std::cout << "Hello, World!" << std::endl;
}

int luaopen_helper(lua_State *l) {
    return 0;
}

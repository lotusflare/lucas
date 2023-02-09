#include "cassandra.h"
#include "errors.c"
#include "logs.c"
#include <luajit-2.1/lua.h>
#include <stdio.h>
#include <string.h>

LucasError *set_authenticator(lua_State *L, int i, CassCluster *cluster)
{
    LucasError *rc = NULL;

    lua_getfield(L, i, "credentials");
    int credentials_index = lua_gettop(L);
    if (lua_isnil(L, credentials_index))
    {
        lucas_log(LOG_WARN, "credentials not provided");
        goto cleanup;
    }

    lua_getfield(L, credentials_index, "username");
    if (lua_isnil(L, lua_gettop(L)))
    {
        rc = lucas_new_errorf("username is missing");
        goto cleanup;
    }
    const char *username = lua_tostring(L, lua_gettop(L));
    lucas_log(LOG_DEBUG, "setting username=%s", username);

    lua_getfield(L, credentials_index, "password");
    if (lua_isnil(L, lua_gettop(L)))
    {
        rc = lucas_new_errorf("password is missing");
        goto cleanup;
    }
    const char *password = lua_tostring(L, lua_gettop(L));
    lucas_log(LOG_DEBUG, "setting password=%s", password);

    cass_cluster_set_credentials(cluster, username, password);
    lucas_log(LOG_INFO, "authenticator configured");

cleanup:
    return rc;
}

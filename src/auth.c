#include "cassandra.h"
#include "errors.c"
#include "logs.c"
#include <luajit-2.1/lua.h>
#include <stdio.h>
#include <string.h>

typedef struct Credentials
{
    const char *password;
    const char *username;
} Credentials;

void on_auth_initial(CassAuthenticator *auth, void *data)
{
    const Credentials *credentials = (const Credentials *)data;
    size_t username_size = strlen(credentials->username);
    size_t password_size = strlen(credentials->password);
    size_t size = username_size + password_size + 2;
    char *response = cass_authenticator_response(auth, size);

    // Credentials are prefixed with '\0'
    response[0] = '\0';
    memcpy(response + 1, credentials->username, username_size);
    response[username_size + 1] = '\0';
    memcpy(response + username_size + 2, credentials->password, password_size);
}

void on_auth_challenge(CassAuthenticator *auth, void *data, const char *token, size_t token_size)
{
}

void on_auth_success(CassAuthenticator *auth, void *data, const char *token, size_t token_size)
{
}

void on_auth_cleanup(CassAuthenticator *auth, void *data)
{
}

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

    lua_getfield(L, credentials_index, "password");
    if (lua_isnil(L, lua_gettop(L)))
    {
        rc = lucas_new_errorf("username is missing");
        goto cleanup;
    }
    const char *password = lua_tostring(L, lua_gettop(L));

    CassAuthenticatorCallbacks auth_callbacks = {on_auth_initial, on_auth_challenge, on_auth_success, on_auth_cleanup};
    Credentials credentials = {username, password};
    CassError err = cass_cluster_set_authenticator_callbacks(cluster, &auth_callbacks, NULL, &credentials);
    if (err != CASS_OK)
    {
        rc = lucas_new_errorf_from_cass_error(err, "failed to set authenticator callbacks");
        goto cleanup;
    }

cleanup:
    return rc;
}

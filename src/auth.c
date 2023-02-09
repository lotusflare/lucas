#include "cassandra.h"
#include <luajit-2.1/lua.h>
#include <stdio.h>
#include <string.h>

typedef struct Credentials_
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

    /* Credentials are prefixed with '\0' */
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

int set_authenticator(lua_State *L, int i, CassCluster *cluster)
{
    CassAuthenticatorCallbacks auth_callbacks = {on_auth_initial, on_auth_challenge, on_auth_success, on_auth_cleanup};
    Credentials credentials = {"cassandra", "cassandra"};
    CassError err = CASS_OK;

    /* Set custom authentication callbacks and credentials */
    err = cass_cluster_set_authenticator_callbacks(cluster, &auth_callbacks, NULL, &credentials);
    return err;
}

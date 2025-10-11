#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "gui/gui_entry.hpp"
#include "libopensubsonic/logger.h"
#include "libopensubsonic/crypto.h"
#include "libopensubsonic/httpclient.h"
#include "libopensubsonic/endpoint_ping.h"
#include "configHandler.h"
#include "player/player.h"
#include "discordrpc.h"

configHandler_config_t* configObj = NULL;
int checkConfigFile();
int validateConnection();

int main(int argc, char** argv) {
    int rc = 0;

    // Read config file
    rc = configHandler_Read(&configObj);
    if (rc != 0) {
        printf("Could not read config file!\n");
        return 1;
    }

    // Run check on the config file
    rc = checkConfigFile();
    if (rc != 0) {
        return 1;
    }

    // Generate opensubsonic login
    opensubsonic_crypto_generateLogin();

    // Check the config for the newly generated crypto salt/token
    if (configObj->internal_opensubsonic_loginSalt == NULL ||
        configObj->internal_opensubsonic_loginToken == NULL) {
        printf("Internal Failure: Could not generate OpenSubsonic Login Salt/Token.\n");
        return 1;
    }

    // Attempt connection
    rc = validateConnection();
    if (rc != 0) {
        return 1;
    }

    // Connection attempt was successful, initialize player
    pthread_t pthr_player;
    pthread_create(&pthr_player, NULL, OSSPlayer_ThrdInit, NULL);

    // Launch Discord RPC
    discordrpc_data* discordrpc = NULL;
    discordrpc_struct_init(&discordrpc);
    discordrpc->state = DISCORDRPC_STATE_IDLE;
    discordrpc_init();
    discordrpc_update(&discordrpc);
    discordrpc_struct_deinit(&discordrpc);

    // Launch QT frontend
    qt_gui_entry(argc, argv);

    // Cleanup and exit
    configHandler_Free(&configObj);
    return 0;
}

int checkConfigFile() {
    // Check for OpenSubsonic username and password
    if (strlen(configObj->opensubsonic_username) == 0 ||
        strlen(configObj->opensubsonic_password) == 0) {
        printf("OpenSubsonic username/password is empty.\n");
        return 1;
    }

    // Check for OpenSubsonic protocol and server
    if (strlen(configObj->opensubsonic_protocol) == 0 ||
        strlen(configObj->opensubsonic_server) == 0) {
        printf("OpenSubsonic protocol/server is empty.\n");
        return 1;
    }

    // Check for ListenBrainz scrobble config
    if (configObj->listenbrainz_enable) {
        if (strlen(configObj->listenbrainz_token) == 0) {
            printf("ListenBrainz scrobbling is enabled, but token is empty.\n");
            return 1;
        }
    }

    // Check for LastFM scrobble config
    if (configObj->lastfm_enable) {
        if (strlen(configObj->lastfm_username) == 0 ||
            strlen(configObj->lastfm_password) == 0) {
            printf("LastFM scrobbling is enabled, but username/password is empty.\n");
            return 1;
        }
        if (strlen(configObj->lastfm_api_key) == 0 ||
            strlen(configObj->lastfm_api_secret) == 0) {
            printf("LastFM scrobbling is enabled, but API key/secret is empty.\n");
            return 1;
        }
        if (strlen(configObj->lastfm_api_session_key) == 0) {
            printf("LastFM scrobbling is enabled, but API session key is empty.\n");
            return 1;
        }
    }

    return 0;
}

int validateConnection() {
    printf("Attempting to connect to /ping at %s://%s...\n", configObj->opensubsonic_protocol, configObj->opensubsonic_server);
    opensubsonic_httpClient_URL_t* pingUrl = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&pingUrl);
    pingUrl->endpoint = OPENSUBSONIC_ENDPOINT_PING;
    opensubsonic_httpClient_formUrl(&pingUrl);
    opensubsonic_ping_struct* OSS_ping_struct;
    opensubsonic_httpClient_fetchResponse(&pingUrl, (void**)&OSS_ping_struct);

    if (!OSS_ping_struct->error) {
        printf("Connection to %s://%s successful.\n", configObj->opensubsonic_protocol, configObj->opensubsonic_server);
        printf("Server: %s %s.\n", OSS_ping_struct->serverType, OSS_ping_struct->serverVersion);
    } else {
        printf("Connection to %s://%s failed:\n", configObj->opensubsonic_protocol, configObj->opensubsonic_server);
        printf("Code %d - %s\n", OSS_ping_struct->errorCode, OSS_ping_struct->errorMessage);
    }

    // NOTE: A little verbose to avoid making another variable to hold error code after freeing structs
    if (!OSS_ping_struct->error) {
        opensubsonic_ping_struct_free(&OSS_ping_struct);
        opensubsonic_httpClient_URL_cleanup(&pingUrl);
        return 0;
    } else {
        opensubsonic_ping_struct_free(&OSS_ping_struct);
        opensubsonic_httpClient_URL_cleanup(&pingUrl);
        return 1;
    }
}

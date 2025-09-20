#include <stdio.h>
#include <stdlib.h>
#include "libopensubsonic/logger.h"
#include "libopensubsonic/crypto.h"
#include "libopensubsonic/httpclient.h"
#include "libopensubsonic/endpoint_ping.h"
#include "configHandler.h"

configHandler_config_t* configObj = NULL;

int main() {
    int rc = 0;

    rc = configHandler_Read(&configObj);
    if (rc != 0) {
        printf("Could not read config file!\n");
        return 1;
    }
    opensubsonic_crypto_generateLogin();

    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_PING;
    opensubsonic_httpClient_formUrl(&url);
    printf("URL: %s\n", url->formedUrl);
    opensubsonic_ping_struct* OSS_ping_struct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&OSS_ping_struct);
    printf("PING OK: %s\n", OSS_ping_struct->status);

    configHandler_Free(&configObj);
    return 0;
}

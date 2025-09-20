#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_scrobble.h"

// Returns 1 if failure occured, else 0
int opensubsonic_scrobble_parse(char* data, opensubsonic_scrobble_struct** scrobbleStruct) {
    // Allocate and initialize
    (*scrobbleStruct) = malloc(sizeof(opensubsonic_scrobble_struct));
    (*scrobbleStruct)->status = NULL;
    (*scrobbleStruct)->errorCode = 0;
    (*scrobbleStruct)->errorMessage = NULL;
    
    // Parse the JSON
    cJSON* root = cJSON_Parse(data);
    if (root == NULL) {
        logger_log_error(__func__, "Error parsing JSON.");
        return 1;
    }

    // Make an object from subsonic-response
    cJSON* subsonic_root = cJSON_GetObjectItemCaseSensitive(root, "subsonic-response");
    if (subsonic_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - subsonic-response does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Check for error
    OSS_Psoj(&(*scrobbleStruct)->status, subsonic_root, "status");
    if (strstr((*scrobbleStruct)->status, "ok") == NULL) {
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*scrobbleStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*scrobbleStruct)->errorMessage, subsonic_error, "message");
        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*scrobbleStruct)->errorCode, (*scrobbleStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }
    
    cJSON_Delete(root);
    return 0;
}

void opensubsonic_scrobble_struct_free(opensubsonic_scrobble_struct** scrobbleStruct) {
    logger_log_general(__func__, "Freeing /scrobble endpoint heap objects.");
    if ((*scrobbleStruct)->status != NULL) { free((*scrobbleStruct)->status); }
    if ((*scrobbleStruct)->errorMessage != NULL) { free((*scrobbleStruct)->errorMessage); }
    if (*scrobbleStruct != NULL) { free(*scrobbleStruct); }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_ping.h"

// Parse the JSON returned from the /rest/ping endpoint
// Returns 1 if failure occured, else 0
int opensubsonic_ping_parse(char* data, opensubsonic_ping_struct** pingStruct) {
    // Allocate on the heap
    *pingStruct = (opensubsonic_ping_struct*)malloc(sizeof(opensubsonic_ping_struct));
    
    // Initialize struct variables
    (*pingStruct)->status = NULL;
    (*pingStruct)->version = NULL;
    (*pingStruct)->serverType = NULL;
    (*pingStruct)->serverVersion = NULL;
    (*pingStruct)->openSubsonicCapable = false;
    (*pingStruct)->error = false;
    (*pingStruct)->errorCode = 0;
    (*pingStruct)->errorMessage = NULL;

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
    
    OSS_Psoj(&(*pingStruct)->status, subsonic_root, "status");
    OSS_Psoj(&(*pingStruct)->version, subsonic_root, "version");
    OSS_Psoj(&(*pingStruct)->serverType, subsonic_root, "type");
    OSS_Psoj(&(*pingStruct)->serverVersion, subsonic_root, "serverVersion");
    OSS_Pboj(&(*pingStruct)->openSubsonicCapable, subsonic_root, "openSubsonic");

    // Check if an error is present
    cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
    if (subsonic_error == NULL) {
        // Error did not occur, return
        cJSON_Delete(root);
        return 0;
    }
    (*pingStruct)->error = true;

    // From this point on, error has occured, capture error information
    OSS_Pioj(&(*pingStruct)->errorCode, subsonic_error, "code");
    OSS_Psoj(&(*pingStruct)->errorMessage, subsonic_error, "message");
    logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*pingStruct)->errorCode, (*pingStruct)->errorMessage);

    cJSON_Delete(root);
    return 1;
}

// Free the dynamically allocated elements of the opensubsonic_ping_struct structure
void opensubsonic_ping_struct_free(opensubsonic_ping_struct** pingStruct) {
    logger_log_general(__func__, "Freeing /ping endpoint heap objects.");
    if ((*pingStruct)->status != NULL) { free((*pingStruct)->status); }
    if ((*pingStruct)->version != NULL) { free((*pingStruct)->version); }
    if ((*pingStruct)->serverType != NULL) { free((*pingStruct)->serverType); }
    if ((*pingStruct)->serverVersion != NULL) { free((*pingStruct)->serverVersion); }
    if ((*pingStruct)->errorMessage != NULL) { free((*pingStruct)->errorMessage); }
    if (*pingStruct != NULL) { free(*pingStruct); }
}

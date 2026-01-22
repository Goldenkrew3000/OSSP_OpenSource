#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getInternetRadioStations.h"

int opensubsonic_getInternetRadioStations_parse(char* data, opensubsonic_getInternetRadioStations_struct** getInternetRadioStationsStruct) {
    // Allocate and initialize struct
    *getInternetRadioStationsStruct = malloc(sizeof(opensubsonic_getInternetRadioStations_struct));
    (*getInternetRadioStationsStruct)->status = NULL;
    (*getInternetRadioStationsStruct)->errorCode = 0;
    (*getInternetRadioStationsStruct)->errorMessage = NULL;
    (*getInternetRadioStationsStruct)->radioStationCount = 0;
    (*getInternetRadioStationsStruct)->radioStations = NULL;

    // Parse JSON
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
    
    OSS_Psoj(&(*getInternetRadioStationsStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getInternetRadioStationsStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getInternetRadioStationsStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getInternetRadioStationsStruct)->errorMessage, subsonic_error, "message");
        
        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getInternetRadioStationsStruct)->errorCode, (*getInternetRadioStationsStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from 'internetRadioStations'
    cJSON* internetRadioStations_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "internetRadioStations");
    if (internetRadioStations_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - internetRadioStations does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from 'internetRadioStation
    cJSON* internetRadioStation_root = cJSON_GetObjectItemCaseSensitive(internetRadioStations_root, "internetRadioStation");
    if (internetRadioStation_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - internetRadioStation does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Get the amount of radio stiations, then allocate and initialize structs
    (*getInternetRadioStationsStruct)->radioStationCount = cJSON_GetArraySize(internetRadioStation_root);
    if ((*getInternetRadioStationsStruct)->radioStationCount != 0) {
        // If there are no radio stations, don't allocate anything, but let the program run through this.
        // Cleanup then happens at the same point, but the following allocation and fetching steps wont happen
        // Basically keeps the code cleaner for this edge case (Although this is probably present in other parts **TODO**)
        (*getInternetRadioStationsStruct)->radioStations = malloc((*getInternetRadioStationsStruct)->radioStationCount * sizeof(opensubsonic_getInternetRadioStations_radioStations_struct));
    }

    for (int i = 0; i < (*getInternetRadioStationsStruct)->radioStationCount; i++) {
        (*getInternetRadioStationsStruct)->radioStations[i].id = NULL;
        (*getInternetRadioStationsStruct)->radioStations[i].name = NULL;
        (*getInternetRadioStationsStruct)->radioStations[i].streamUrl = NULL;
    }

    for (int i = 0; i < (*getInternetRadioStationsStruct)->radioStationCount; i++) {
        cJSON* curr_idx_root = cJSON_GetArrayItem(internetRadioStation_root, i);
        if (curr_idx_root != NULL) {
            OSS_Psoj(&(*getInternetRadioStationsStruct)->radioStations[i].id, curr_idx_root, "id");
            OSS_Psoj(&(*getInternetRadioStationsStruct)->radioStations[i].name, curr_idx_root, "name");
            OSS_Psoj(&(*getInternetRadioStationsStruct)->radioStations[i].streamUrl, curr_idx_root, "streamUrl");
        }
    }

    cJSON_Delete(root);
    return 0;
}

void opensubsonic_getInternetRadioStations_struct_free(opensubsonic_getInternetRadioStations_struct** getInternetRadioStationsStruct) {
    logger_log_general(__func__, "Freeing /getInternetRadioStations endpoint heap objects.");
    if ((*getInternetRadioStationsStruct)->status != NULL) { free((*getInternetRadioStationsStruct)->status); }
    if ((*getInternetRadioStationsStruct)->errorMessage != NULL) { free((*getInternetRadioStationsStruct)->errorMessage); }
    for (int i = 0; i < (*getInternetRadioStationsStruct)->radioStationCount; i++) {
        if ((*getInternetRadioStationsStruct)->radioStations[i].id != NULL) { free((*getInternetRadioStationsStruct)->radioStations[i].id); }
        if ((*getInternetRadioStationsStruct)->radioStations[i].name != NULL) { free((*getInternetRadioStationsStruct)->radioStations[i].name); }
        if ((*getInternetRadioStationsStruct)->radioStations[i].streamUrl != NULL) { free((*getInternetRadioStationsStruct)->radioStations[i].streamUrl); }
    }
    if ((*getInternetRadioStationsStruct)->radioStations != NULL) { free((*getInternetRadioStationsStruct)->radioStations); }
    if (*getInternetRadioStationsStruct != NULL) { free(*getInternetRadioStationsStruct); }
}

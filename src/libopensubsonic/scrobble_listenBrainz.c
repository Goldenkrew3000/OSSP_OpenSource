#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "endpoint_getSong.h"
#include "httpclient.h"
#include "scrobble_listenBrainz.h"
#include "../DarwinHttpClient.h"

const char* listenBrainzScrobbleURL = "https://api.listenbrainz.org/1/submit-listens";

void opensubsonic_scrobble_listenBrainz(bool finalize, opensubsonic_getSong_struct* songStruct) {
    if (finalize) {
        logger_log_general(__func__, "Performing final scrobble to ListenBrainz.");
    } else {
        logger_log_general(__func__, "Performing in-progress scrobble to ListenBrainz.");
    }
    
    // Fetch the current UNIX timestamp
    time_t currentTime;
    currentTime = time(NULL);
    
    // Form the JSON body
    cJSON* rootObj = cJSON_CreateObject();
    
    // Add ["listen_type"]
    if (finalize) {
        cJSON* listenTypeObj = cJSON_CreateString("single");
        cJSON_AddItemToObject(rootObj, "listen_type", listenTypeObj);
    } else {
        cJSON* listenTypeObj = cJSON_CreateString("playing_now");
        cJSON_AddItemToObject(rootObj, "listen_type", listenTypeObj);
    }
    
    // Add ["payload"]
    cJSON* payloadObj = cJSON_CreateArray();
    cJSON_AddItemToObject(rootObj, "payload", payloadObj);
    
    // Add ["payload"][]
    cJSON* payloadContainedObj = cJSON_CreateObject();
    cJSON_AddItemToArray(payloadObj, payloadContainedObj);
    
    // Add ["listened_at"] if 'finalize' is true
    if (finalize) {
        cJSON* listenedAtObj = cJSON_CreateNumber((long)currentTime);
        cJSON_AddItemToObject(payloadContainedObj, "listened_at", listenedAtObj);
    }
    
    // Add ["track_metadata"]
    cJSON* trackMetadataObj = cJSON_CreateObject();
    cJSON_AddItemToObject(payloadContainedObj, "track_metadata", trackMetadataObj);
    
    // Add ["track_metadata"]["additional_info"]
    cJSON* additionalInfoObj = cJSON_CreateObject();
    cJSON_AddItemToObject(trackMetadataObj, "additional_info", additionalInfoObj);
    
    // Add ["track_metadata"]["additional_info"]["media_player"]
    cJSON* mediaPlayerObj = cJSON_CreateString("OSSP");
    cJSON_AddItemToObject(additionalInfoObj, "media_player", mediaPlayerObj);
    
    // Add ["track_metadata"]["additional_info"]["submission_client"]
    cJSON* submissionClientObj = cJSON_CreateString("OSSP ListenBrainz Scrobbler");
    cJSON_AddItemToObject(additionalInfoObj, "submission_client", submissionClientObj);
    
    // Add ["track_metadata"]["additional_info"]["submission_client_version"]
    cJSON* submissionClientVersionObj = cJSON_CreateString("1.0");
    cJSON_AddItemToObject(additionalInfoObj, "submission_client_version", submissionClientVersionObj);
    
    // Add ["track_metadata"]["artist_name"]
    if (songStruct->artist != NULL) {
        cJSON* artistNameObj = cJSON_CreateString(songStruct->artist);
        cJSON_AddItemToObject(trackMetadataObj, "artist_name", artistNameObj);
    } else {
        printf("[ListenBrainz Scrobbler] Song Artist ([\"artist\"]) is null in songStruct\n");
    }
    
    // Add ["track_metadata"]["track_name"]
    if (songStruct->title != NULL) {
        cJSON* artistTitleObj = cJSON_CreateString(songStruct->title);
        cJSON_AddItemToObject(trackMetadataObj, "track_name", artistTitleObj);
    } else {
        printf("[ListenBrainz Scrobbler] Song Title ([\"title\"]) is null in songStruct\n");
    }
    
    // Add ["track_metadata"]["release_name"]
    if (songStruct->album != NULL) {
        cJSON* artistReleaseObj = cJSON_CreateString(songStruct->artist);
        cJSON_AddItemToObject(trackMetadataObj, "release_name", artistReleaseObj);
    } else {
        printf("[ListenBrainz Scrobbler] Song Album ([\"album\"]) is null in songStruct\n");
    }
    
    // Print the assembled JSON
    char* payload = cJSON_PrintUnformatted(rootObj);
    cJSON_Delete(rootObj);
    
    // Send payload and receive JSON response
    opensubsonic_httpClientRequest_t* httpReq;
    opensubsonic_httpClient_prepareRequest(&httpReq);
    
    httpReq->requestUrl = strdup(listenBrainzScrobbleURL);
    httpReq->requestBody = strdup(payload);
    free(payload);
    httpReq->isBodyRequired = true;
    httpReq->scrobbler = SCROBBLER_LISTENBRAINZ;
    httpReq->method = HTTP_METHOD_POST;
    opensubsonic_httpClient_request(&httpReq);
    
    // Check response
    if (httpReq->responseCode != HTTP_CODE_SUCCESS) {
        logger_log_error(__func__, "HTTP POST did not return success (%d).", httpReq->responseCode);
        opensubsonic_httpClient_cleanup(&httpReq);
        // TODO return error
    }
    
    cJSON* responseObj = cJSON_Parse(httpReq->responseMsg);
    opensubsonic_httpClient_cleanup(&httpReq);
    if (responseObj == NULL) {
        logger_log_error(__func__, "Error parsing JSON.");
        // TODO return error
    }
    
    cJSON* status = cJSON_GetObjectItemCaseSensitive(responseObj, "status");
    if (cJSON_IsString(status) && status->valuestring != NULL) {
        if (strcmp(status->valuestring, "ok") == 0) {
            if (finalize) {
                logger_log_general(__func__, "Final scrobble was successful.");
            } else {
                logger_log_general(__func__, "In progress scrobble was successful.");
            }
        } else {
            logger_log_error(__func__, "Something went wrong scrobbling to ListenBrainz.");
        }
    } else {
        logger_log_error(__func__, "Something went wrong scrobbling to ListenBrainz.");
    }
    
    cJSON_Delete(responseObj);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getLyricsBySongId.h"

// Returns 1 if failure occured, else 0
int opensubsonic_getLyricsBySongId_parse(char* data, opensubsonic_getLyricsBySongId_struct** getLyricsBySongIdStruct) {
    // Allocate struct
    *getLyricsBySongIdStruct = malloc(sizeof(opensubsonic_getLyricsBySongId_struct));
    
    // Initialize struct variables
    (*getLyricsBySongIdStruct)->status = NULL;
    (*getLyricsBySongIdStruct)->errorCode = 0;
    (*getLyricsBySongIdStruct)->errorMessage = NULL;
    (*getLyricsBySongIdStruct)->displayArtist = NULL;
    (*getLyricsBySongIdStruct)->displayTitle = NULL;
    (*getLyricsBySongIdStruct)->lyricsAmount = 0;
    (*getLyricsBySongIdStruct)->lyrics = NULL;
    
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
    
    OSS_Psoj(&(*getLyricsBySongIdStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getLyricsBySongIdStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getLyricsBySongIdStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getLyricsBySongIdStruct)->errorMessage, subsonic_error, "message");

        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getLyricsBySongIdStruct)->errorCode, (*getLyricsBySongIdStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }
    
    // Make an object from 'lyricsList'
    cJSON* lyricsList_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "lyricsList");
    if (lyricsList_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - lyricsList does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Make an object from 'structuredLyrics'
    cJSON* structuredLyrics_root = cJSON_GetObjectItemCaseSensitive(lyricsList_root, "structuredLyrics");
    if (structuredLyrics_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - structuredLyrics does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Make an object from the first index of 'structuredLyrics'
    cJSON* structuredLyrics_idx0_root = cJSON_GetArrayItem(structuredLyrics_root, 0);
    if (structuredLyrics_idx0_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - structuredLyrics (idx 0) does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    OSS_Psoj(&(*getLyricsBySongIdStruct)->displayArtist, structuredLyrics_idx0_root, "displayArtist");
    OSS_Psoj(&(*getLyricsBySongIdStruct)->displayTitle, structuredLyrics_idx0_root, "displayTitle");
    
    // Make an object from 'line'
    cJSON* line_root = cJSON_GetObjectItemCaseSensitive(structuredLyrics_idx0_root, "line");
    if (line_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - line does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Count amount of objects in 'line' and allocate memory
    (*getLyricsBySongIdStruct)->lyricsAmount = cJSON_GetArraySize(line_root);
    (*getLyricsBySongIdStruct)->lyrics = (opensubsonic_getLyricsBySongId_lyric_struct*)malloc((*getLyricsBySongIdStruct)->lyricsAmount * sizeof(opensubsonic_getLyricsBySongId_lyric_struct));
    
    // Initialize variables
    for (int i = 0; i < (*getLyricsBySongIdStruct)->lyricsAmount; i++) {
        (*getLyricsBySongIdStruct)->lyrics[i].data = NULL;
        (*getLyricsBySongIdStruct)->lyrics[i].offset = 0;
    }
    
    // Extract lyrics
    for (int i = 0; i < (*getLyricsBySongIdStruct)->lyricsAmount; i++) {
        cJSON* curr_idx_root = cJSON_GetArrayItem(line_root, i);
        if (curr_idx_root != NULL) {
            OSS_Psoj(&(*getLyricsBySongIdStruct)->lyrics[i].data, curr_idx_root, "value");
            OSS_Ploj(&(*getLyricsBySongIdStruct)->lyrics[i].offset, curr_idx_root, "start");
        }
    }
    
    cJSON_Delete(root);
    return 0;
}

void opensubsonic_getLyricsBySongId_struct_free(opensubsonic_getLyricsBySongId_struct** getLyricsBySongIdStruct) {
    logger_log_general(__func__, "Freeing /getLyricsBySongId endpoint heap objects.");
    if ((*getLyricsBySongIdStruct)->status != NULL) { free((*getLyricsBySongIdStruct)->status); }
    if ((*getLyricsBySongIdStruct)->errorMessage != NULL) { free((*getLyricsBySongIdStruct)->errorMessage); }
    if ((*getLyricsBySongIdStruct)->displayArtist != NULL) { free((*getLyricsBySongIdStruct)->displayArtist); }
    if ((*getLyricsBySongIdStruct)->displayTitle != NULL) { free((*getLyricsBySongIdStruct)->displayTitle); }
    for (int i = 0; i < (*getLyricsBySongIdStruct)->lyricsAmount; i++) {
        if ((*getLyricsBySongIdStruct)->lyrics[i].data != NULL) { free((*getLyricsBySongIdStruct)->lyrics[i].data); }
    }
    if ((*getLyricsBySongIdStruct)->lyrics != NULL) { free((*getLyricsBySongIdStruct)->lyrics); }
    if (*getLyricsBySongIdStruct != NULL) { free(*getLyricsBySongIdStruct); }
}

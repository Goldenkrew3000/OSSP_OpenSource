#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getPlaylists.h"

// Parse the JSON returned from the /rest/getPlaylists endpoint
// Returns 1 if failure occured, else 0
int opensubsonic_getPlaylists_parse(char* data, opensubsonic_getPlaylists_struct** getPlaylistsStruct) {
    // Allocate struct
    *getPlaylistsStruct = (opensubsonic_getPlaylists_struct*)malloc(sizeof(opensubsonic_getPlaylists_struct));
    
    // Initialize struct variables
    (*getPlaylistsStruct)->status = NULL;
    (*getPlaylistsStruct)->errorCode = 0;
    (*getPlaylistsStruct)->errorMessage = NULL;
    (*getPlaylistsStruct)->playlistCount = 0;
    (*getPlaylistsStruct)->playlists = NULL;
    
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
    
    OSS_Psoj(&(*getPlaylistsStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getPlaylistsStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getPlaylistsStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getPlaylistsStruct)->errorMessage, subsonic_error, "message");
        
        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getPlaylistsStruct)->errorCode, (*getPlaylistsStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }
    
    // Make an object from playlists
    cJSON* playlists_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "playlists");
    if (playlists_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - playlists does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Make an object from playlist
    cJSON* playlist_root = cJSON_GetObjectItemCaseSensitive(playlists_root, "playlist");
    if (playlist_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - playlist does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Get the amount of playlists
    (*getPlaylistsStruct)->playlistCount = cJSON_GetArraySize(playlist_root);
    
    // Allocate memory for opensubsonic_getPlaylists_playlist_struct inside opensubsonic_getPlaylists_struct
    (*getPlaylistsStruct)->playlists = (opensubsonic_getPlaylists_playlist_struct*)malloc((*getPlaylistsStruct)->playlistCount * sizeof(opensubsonic_getPlaylists_playlist_struct));
    
    // Initialize struct variables
    for (int i = 0; i < (*getPlaylistsStruct)->playlistCount; i++) {
        (*getPlaylistsStruct)->playlists[i].id = NULL;
        (*getPlaylistsStruct)->playlists[i].name = NULL;
        (*getPlaylistsStruct)->playlists[i].songCount = 0;
        (*getPlaylistsStruct)->playlists[i].duration = 0;
        (*getPlaylistsStruct)->playlists[i].owner = NULL;
        (*getPlaylistsStruct)->playlists[i].coverArt = NULL;
    }
    
    // Extract the data from the playlist array
    for (int i = 0; i < (*getPlaylistsStruct)->playlistCount; i++) {
        cJSON* current_index_root = cJSON_GetArrayItem(playlist_root, i);
        if (current_index_root != NULL) {
            // Fetch playlist information
            OSS_Psoj(&(*getPlaylistsStruct)->playlists[i].id, current_index_root, "id");
            OSS_Psoj(&(*getPlaylistsStruct)->playlists[i].name, current_index_root, "name");
            OSS_Pioj(&(*getPlaylistsStruct)->playlists[i].songCount, current_index_root, "songCount");
            OSS_Ploj(&(*getPlaylistsStruct)->playlists[i].duration, current_index_root, "duration");
            OSS_Psoj(&(*getPlaylistsStruct)->playlists[i].owner, current_index_root, "owner");
            OSS_Psoj(&(*getPlaylistsStruct)->playlists[i].coverArt, current_index_root, "coverArt");
        }
    }
    
    cJSON_Delete(root);
    return 0;
}

void opensubsonic_getPlaylists_struct_free(opensubsonic_getPlaylists_struct** getPlaylistsStruct) {
    logger_log_general(__func__, "Freeing /getPlaylists endpoint heap objects.");
    if ((*getPlaylistsStruct)->status != NULL) { free((*getPlaylistsStruct)->status); }
    if ((*getPlaylistsStruct)->errorMessage != NULL) { free((*getPlaylistsStruct)->errorMessage); }
    for (size_t i = 0; i < (*getPlaylistsStruct)->playlistCount; i++) {
        if ((*getPlaylistsStruct)->playlists[i].id != NULL) { free((*getPlaylistsStruct)->playlists[i].id); }
        if ((*getPlaylistsStruct)->playlists[i].name != NULL) { free((*getPlaylistsStruct)->playlists[i].name); }
        if ((*getPlaylistsStruct)->playlists[i].owner != NULL) { free((*getPlaylistsStruct)->playlists[i].owner); }
        if ((*getPlaylistsStruct)->playlists[i].coverArt != NULL) { free((*getPlaylistsStruct)->playlists[i].coverArt); }
    }
    if ((*getPlaylistsStruct)->playlists != NULL) { free((*getPlaylistsStruct)->playlists); }
    if (*getPlaylistsStruct != NULL) { free(*getPlaylistsStruct); }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getPlaylist.h"

int opensubsonic_getPlaylist_parse(char* data, opensubsonic_getPlaylist_struct** getPlaylistStruct) {
    // Allocate struct
    *getPlaylistStruct = (opensubsonic_getPlaylist_struct*)malloc(sizeof(opensubsonic_getPlaylist_struct));
    
    // Initialize struct variables
    (*getPlaylistStruct)->status = NULL;
    (*getPlaylistStruct)->errorCode = 0;
    (*getPlaylistStruct)->errorMessage = NULL;
    (*getPlaylistStruct)->id = NULL;
    (*getPlaylistStruct)->name = NULL;
    (*getPlaylistStruct)->owner = NULL;
    (*getPlaylistStruct)->isPublic = false;
    (*getPlaylistStruct)->created = NULL;
    (*getPlaylistStruct)->changed = NULL;
    (*getPlaylistStruct)->songCount = 0;
    (*getPlaylistStruct)->duration = 0;
    (*getPlaylistStruct)->songs = NULL;
    
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
    
    OSS_Psoj(&(*getPlaylistStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getPlaylistStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getPlaylistStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getPlaylistStruct)->errorMessage, subsonic_error, "message");
        
        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getPlaylistStruct)->errorCode, (*getPlaylistStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }
    
    // Make an object from 'playlist'
    cJSON* playlist_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "playlist");
    if (playlist_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - playlist does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    OSS_Psoj(&(*getPlaylistStruct)->id, playlist_root, "id");
    OSS_Psoj(&(*getPlaylistStruct)->name, playlist_root, "name");
    OSS_Psoj(&(*getPlaylistStruct)->owner, playlist_root, "owner");
    OSS_Pboj(&(*getPlaylistStruct)->isPublic, playlist_root, "public");
    OSS_Psoj(&(*getPlaylistStruct)->created, playlist_root, "created");
    OSS_Psoj(&(*getPlaylistStruct)->changed, playlist_root, "changed");
    OSS_Ploj(&(*getPlaylistStruct)->duration, playlist_root, "duration");

    // Make an object from 'entry'
    cJSON* entry_root = cJSON_GetObjectItemCaseSensitive(playlist_root, "entry");
    if (entry_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - entry does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Get the amount of songs in the playlist, and allocate memory
    (*getPlaylistStruct)->songCount = cJSON_GetArraySize(entry_root);
    (*getPlaylistStruct)->songs = (opensubsonic_getPlaylist_songs_struct*)malloc((*getPlaylistStruct)->songCount * sizeof(opensubsonic_getPlaylist_songs_struct));
    
    // Initialize struct variables
    for (int i = 0; i < (*getPlaylistStruct)->songCount; i++) {
        (*getPlaylistStruct)->songs[i].id = NULL;
        (*getPlaylistStruct)->songs[i].parent = NULL;
        (*getPlaylistStruct)->songs[i].title = NULL;
        (*getPlaylistStruct)->songs[i].albumId = NULL;
        (*getPlaylistStruct)->songs[i].album = NULL;
        (*getPlaylistStruct)->songs[i].artistId = NULL;
        (*getPlaylistStruct)->songs[i].artist = NULL;
        (*getPlaylistStruct)->songs[i].coverArt = NULL;
        (*getPlaylistStruct)->songs[i].duration = 0;
        (*getPlaylistStruct)->songs[i].bitRate = 0;
        (*getPlaylistStruct)->songs[i].bitDepth = 0;
        (*getPlaylistStruct)->songs[i].samplingRate = 0;
        (*getPlaylistStruct)->songs[i].channelCount = 0;
        (*getPlaylistStruct)->songs[i].userRating = 0;
        (*getPlaylistStruct)->songs[i].track = 0;
        (*getPlaylistStruct)->songs[i].year = 0;
        (*getPlaylistStruct)->songs[i].genre = NULL;
        (*getPlaylistStruct)->songs[i].size = 0;
        (*getPlaylistStruct)->songs[i].discNumber = 0;
    }
    
    for (int i = 0; i < (*getPlaylistStruct)->songCount; i++) {
        cJSON* curr_idx_root = cJSON_GetArrayItem(entry_root, i);
        if (curr_idx_root != NULL) {
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].id, curr_idx_root, "id");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].parent, curr_idx_root, "parent");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].title, curr_idx_root, "title");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].albumId, curr_idx_root, "albumId");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].album, curr_idx_root, "album");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].artistId, curr_idx_root, "artistId");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].artist, curr_idx_root, "artist");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].coverArt, curr_idx_root, "coverArt");
            OSS_Ploj(&(*getPlaylistStruct)->songs[i].duration, curr_idx_root, "duration");
            OSS_Pioj(&(*getPlaylistStruct)->songs[i].bitRate, curr_idx_root, "bitRate");
            OSS_Pioj(&(*getPlaylistStruct)->songs[i].bitDepth, curr_idx_root, "bitDepth");
            OSS_Ploj(&(*getPlaylistStruct)->songs[i].samplingRate, curr_idx_root, "samplingRate");
            OSS_Pioj(&(*getPlaylistStruct)->songs[i].channelCount, curr_idx_root, "channelCount");
            OSS_Pioj(&(*getPlaylistStruct)->songs[i].userRating, curr_idx_root, "userRating");
            OSS_Pioj(&(*getPlaylistStruct)->songs[i].track, curr_idx_root, "track");
            OSS_Pioj(&(*getPlaylistStruct)->songs[i].year, curr_idx_root, "year");
            OSS_Psoj(&(*getPlaylistStruct)->songs[i].genre, curr_idx_root, "genre");
            OSS_Ploj(&(*getPlaylistStruct)->songs[i].size, curr_idx_root, "size");
            OSS_Pioj(&(*getPlaylistStruct)->songs[i].discNumber, curr_idx_root, "discNumber");
        }
    }
    
    cJSON_Delete(root);
    return 0;
}

void opensubsonic_getPlaylist_struct_free(opensubsonic_getPlaylist_struct** getPlaylistStruct) {
    logger_log_general(__func__, "Freeing /getPlaylist endpoint heap objects.");
    if ((*getPlaylistStruct)->status != NULL) { free((*getPlaylistStruct)->status); }
    if ((*getPlaylistStruct)->errorMessage != NULL) { free((*getPlaylistStruct)->errorMessage); }
    if ((*getPlaylistStruct)->id != NULL) { free((*getPlaylistStruct)->id); }
    if ((*getPlaylistStruct)->name != NULL) { free((*getPlaylistStruct)->name); }
    if ((*getPlaylistStruct)->owner != NULL) { free((*getPlaylistStruct)->owner); }
    if ((*getPlaylistStruct)->created != NULL) { free((*getPlaylistStruct)->created); }
    if ((*getPlaylistStruct)->changed != NULL) { free((*getPlaylistStruct)->changed); }
    for (int i = 0; i < (*getPlaylistStruct)->songCount; i++) {
        if((*getPlaylistStruct)->songs[i].id != NULL) { free((*getPlaylistStruct)->songs[i].id); }
        if((*getPlaylistStruct)->songs[i].parent != NULL) { free((*getPlaylistStruct)->songs[i].parent); }
        if((*getPlaylistStruct)->songs[i].title != NULL) { free((*getPlaylistStruct)->songs[i].title); }
        if((*getPlaylistStruct)->songs[i].albumId != NULL) { free((*getPlaylistStruct)->songs[i].albumId); }
        if((*getPlaylistStruct)->songs[i].album != NULL) { free((*getPlaylistStruct)->songs[i].album); }
        if((*getPlaylistStruct)->songs[i].artistId != NULL) { free((*getPlaylistStruct)->songs[i].artistId); }
        if((*getPlaylistStruct)->songs[i].artist != NULL) { free((*getPlaylistStruct)->songs[i].artist); }
        if((*getPlaylistStruct)->songs[i].coverArt != NULL) { free((*getPlaylistStruct)->songs[i].coverArt); }
        if((*getPlaylistStruct)->songs[i].genre != NULL) { free((*getPlaylistStruct)->songs[i].genre); }
    }
    if ((*getPlaylistStruct)->songs != NULL) { free((*getPlaylistStruct)->songs); }
    if (*getPlaylistStruct != NULL) { free(*getPlaylistStruct); }
}

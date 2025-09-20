#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getAlbumList.h"

// Returns 1 if failure occured, else 0
int opensubsonic_getAlbumList_parse(char* data, opensubsonic_getAlbumList_struct** getAlbumListStruct) {
    // Allocate struct and initialize variables
    (*getAlbumListStruct) = malloc(sizeof(opensubsonic_getAlbumList_struct));
    (*getAlbumListStruct)->status = NULL;
    (*getAlbumListStruct)->errorCode = 0;
    (*getAlbumListStruct)->errorMessage = NULL;
    (*getAlbumListStruct)->albumCount = 0;
    (*getAlbumListStruct)->albums = NULL;
    
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

    OSS_Psoj(&(*getAlbumListStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getAlbumListStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getAlbumListStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getAlbumListStruct)->errorMessage, subsonic_error, "message");

        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getAlbumListStruct)->errorCode, (*getAlbumListStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }
    
    cJSON* albumList_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "albumList");
    if (albumList_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - albumList does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    cJSON* album_root = cJSON_GetObjectItemCaseSensitive(albumList_root, "album");
    if (album_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - album does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    (*getAlbumListStruct)->albumCount = cJSON_GetArraySize(album_root);
    
    // Allocate and initialize
    (*getAlbumListStruct)->albums = malloc((*getAlbumListStruct)->albumCount * sizeof(opensubsonic_getAlbumList_album_struct));
    for (int i = 0; i < (*getAlbumListStruct)->albumCount; i++) {
        (*getAlbumListStruct)->albums[i].id = NULL;
        (*getAlbumListStruct)->albums[i].parent = NULL;
        (*getAlbumListStruct)->albums[i].album = NULL;
        (*getAlbumListStruct)->albums[i].title = NULL;
        (*getAlbumListStruct)->albums[i].name = NULL;
        (*getAlbumListStruct)->albums[i].coverArt = NULL;
        (*getAlbumListStruct)->albums[i].songCount = 0;
        (*getAlbumListStruct)->albums[i].created = NULL;
        (*getAlbumListStruct)->albums[i].duration = 0;
        (*getAlbumListStruct)->albums[i].playCount = 0;
        (*getAlbumListStruct)->albums[i].artistId = NULL;
        (*getAlbumListStruct)->albums[i].artist = NULL;
        (*getAlbumListStruct)->albums[i].year = 0;
        (*getAlbumListStruct)->albums[i].genre = NULL;
        
    }
    
    for (int i = 0; i < (*getAlbumListStruct)->albumCount; i++) {
        cJSON* array_album_root = cJSON_GetArrayItem(album_root, i);
        if (array_album_root != NULL) {
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].id, array_album_root, "id");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].parent, array_album_root, "parent");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].album, array_album_root, "album");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].title, array_album_root, "title");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].name, array_album_root, "name");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].coverArt, array_album_root, "coverArt");
            OSS_Pioj(&(*getAlbumListStruct)->albums[i].songCount, array_album_root, "songCount");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].created, array_album_root, "created");
            OSS_Ploj(&(*getAlbumListStruct)->albums[i].duration, array_album_root, "duration");
            OSS_Pioj(&(*getAlbumListStruct)->albums[i].playCount, array_album_root, "playCount");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].artistId, array_album_root, "artistId");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].artist, array_album_root, "artist");
            OSS_Pioj(&(*getAlbumListStruct)->albums[i].year, array_album_root, "year");
            OSS_Psoj(&(*getAlbumListStruct)->albums[i].genre, array_album_root, "genre");
        }
    }
    
    cJSON_Delete(root);
    return 0;
}

void opensubsonic_getAlbumList_struct_free(opensubsonic_getAlbumList_struct** getAlbumListStruct) {
    logger_log_general(__func__, "Freeing /getAlbumList endpoint heap objects.");
    if ((*getAlbumListStruct)->status != NULL) { free((*getAlbumListStruct)->status); }
    if ((*getAlbumListStruct)->errorMessage != NULL) { free((*getAlbumListStruct)->errorMessage); }
    for (int i = 0; i < (*getAlbumListStruct)->albumCount; i++) {
        if ((*getAlbumListStruct)->albums[i].id != NULL) { free((*getAlbumListStruct)->albums[i].id); }
        if ((*getAlbumListStruct)->albums[i].parent != NULL) { free((*getAlbumListStruct)->albums[i].parent); }
        if ((*getAlbumListStruct)->albums[i].album != NULL) { free((*getAlbumListStruct)->albums[i].album); }
        if ((*getAlbumListStruct)->albums[i].title != NULL) { free((*getAlbumListStruct)->albums[i].title); }
        if ((*getAlbumListStruct)->albums[i].name != NULL) { free((*getAlbumListStruct)->albums[i].name); }
        if ((*getAlbumListStruct)->albums[i].coverArt != NULL) { free((*getAlbumListStruct)->albums[i].coverArt); }
        if ((*getAlbumListStruct)->albums[i].created != NULL) { free((*getAlbumListStruct)->albums[i].created); }
        if ((*getAlbumListStruct)->albums[i].artistId != NULL) { free((*getAlbumListStruct)->albums[i].artistId); }
        if ((*getAlbumListStruct)->albums[i].artist != NULL) { free((*getAlbumListStruct)->albums[i].artist); }
        if ((*getAlbumListStruct)->albums[i].genre != NULL) { free((*getAlbumListStruct)->albums[i].genre); }
    }
    if ((*getAlbumListStruct)->albums != NULL) { free((*getAlbumListStruct)->albums); }
    if (*getAlbumListStruct != NULL) { free(*getAlbumListStruct); }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getArtist.h"

// Parse the JSON returned from the /rest/getArtist endpoint
// Returns 1 if failure occured, else 0
int opensubsonic_getArtist_parse(char* data, opensubsonic_getArtist_struct** getArtistStruct) {
    // Allocate struct
    *getArtistStruct = malloc(sizeof(opensubsonic_getArtist_struct));
    
    // Initialize struct variables
    (*getArtistStruct)->status = NULL;
    (*getArtistStruct)->errorCode = 0;
    (*getArtistStruct)->errorMessage = NULL;
    (*getArtistStruct)->artistId = NULL;
    (*getArtistStruct)->artistName = NULL;
    (*getArtistStruct)->coverArt = NULL;
    (*getArtistStruct)->albumCount = 0;
    (*getArtistStruct)->userRating = 0;
    (*getArtistStruct)->artistImageUrl = NULL;
    (*getArtistStruct)->starred = NULL;
    (*getArtistStruct)->albums = NULL;

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

    OSS_Psoj(&(*getArtistStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getArtistStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getArtistStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getArtistStruct)->errorMessage, subsonic_error, "message");

        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getArtistStruct)->errorCode, (*getArtistStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from artist
    cJSON* artist_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "artist");
    if (artist_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - artist does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    OSS_Psoj(&(*getArtistStruct)->artistId, artist_root, "id");
    OSS_Psoj(&(*getArtistStruct)->artistName, artist_root, "name");
    OSS_Psoj(&(*getArtistStruct)->coverArt, artist_root, "coverArt");
    OSS_Pioj(&(*getArtistStruct)->userRating, artist_root, "userRating");
    OSS_Psoj(&(*getArtistStruct)->artistImageUrl, artist_root, "artistImageUrl");
    OSS_Psoj(&(*getArtistStruct)->starred, artist_root, "starred");
    
    // Make an object from album
    cJSON* album_root = cJSON_GetObjectItemCaseSensitive(artist_root, "album");
    if (album_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - album does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Count the amount of albums
    (*getArtistStruct)->albumCount = cJSON_GetArraySize(album_root);
    
    // Allocate memory for albums
    (*getArtistStruct)->albums = malloc((*getArtistStruct)->albumCount * sizeof(opensubsonic_getArtist_album_struct));

    // Initialize variables
    for (int i = 0; i < (*getArtistStruct)->albumCount; i++) {
        (*getArtistStruct)->albums[i].id = NULL;
        (*getArtistStruct)->albums[i].parent = NULL;
        (*getArtistStruct)->albums[i].title = NULL;
        (*getArtistStruct)->albums[i].name = NULL;
        (*getArtistStruct)->albums[i].coverArt = NULL;
        (*getArtistStruct)->albums[i].songCount = 0;
        (*getArtistStruct)->albums[i].created = NULL;
        (*getArtistStruct)->albums[i].duration = 0;
        (*getArtistStruct)->albums[i].playCount = 0;
        (*getArtistStruct)->albums[i].artistId = NULL;
        (*getArtistStruct)->albums[i].artist = NULL;
        (*getArtistStruct)->albums[i].year = 0;
        (*getArtistStruct)->albums[i].genre = NULL;
        (*getArtistStruct)->albums[i].userRating = 0;
        (*getArtistStruct)->albums[i].starred = NULL;
    }
    
    // Go through the album array
    for (int i = 0; i < (*getArtistStruct)->albumCount; i++) {
        cJSON* array_album_root = cJSON_GetArrayItem(album_root, i);
        if (array_album_root != NULL) {
            OSS_Psoj(&(*getArtistStruct)->albums[i].id, array_album_root, "id");
            OSS_Psoj(&(*getArtistStruct)->albums[i].parent, array_album_root, "parent");
            OSS_Psoj(&(*getArtistStruct)->albums[i].title, array_album_root, "title");
            OSS_Psoj(&(*getArtistStruct)->albums[i].name, array_album_root, "name");
            OSS_Psoj(&(*getArtistStruct)->albums[i].coverArt, array_album_root, "coverArt");
            OSS_Pioj(&(*getArtistStruct)->albums[i].songCount, array_album_root, "songCount");
            OSS_Psoj(&(*getArtistStruct)->albums[i].created, array_album_root, "created");
            OSS_Ploj(&(*getArtistStruct)->albums[i].duration, array_album_root, "duration");
            OSS_Pioj(&(*getArtistStruct)->albums[i].playCount, array_album_root, "playCount");
            OSS_Psoj(&(*getArtistStruct)->albums[i].artistId, array_album_root, "artistId");
            OSS_Psoj(&(*getArtistStruct)->albums[i].artist, array_album_root, "artist");
            OSS_Pioj(&(*getArtistStruct)->albums[i].year, array_album_root, "year");
            OSS_Psoj(&(*getArtistStruct)->albums[i].genre, array_album_root, "genre");
            OSS_Pioj(&(*getArtistStruct)->albums[i].userRating, array_album_root, "userRating");
            OSS_Psoj(&(*getArtistStruct)->albums[i].starred, array_album_root, "starred");
        }
    }

    cJSON_Delete(root);
    return 0;
}

// Free the dynamically allocated elements of the opensubsonic_getArtist_struct structure and the opensubsonic_getArtist_album_struct array structs.
void opensubsonic_getArtist_struct_free(opensubsonic_getArtist_struct** getArtistStruct) {
    logger_log_general(__func__, "Freeing /getArtist endpoint heap objects.");
    if((*getArtistStruct)->status != NULL) { free((*getArtistStruct)->status); }
    if((*getArtistStruct)->errorMessage != NULL) { free((*getArtistStruct)->errorMessage); }
    if((*getArtistStruct)->artistId != NULL) { free((*getArtistStruct)->artistId); }
    if((*getArtistStruct)->artistName != NULL) { free((*getArtistStruct)->artistName); }
    if((*getArtistStruct)->coverArt != NULL) { free((*getArtistStruct)->coverArt); }
    if((*getArtistStruct)->artistImageUrl != NULL) { free((*getArtistStruct)->artistImageUrl); }
    if((*getArtistStruct)->starred != NULL) { free((*getArtistStruct)->starred); }
    for (int i = 0; i < (*getArtistStruct)->albumCount; i++) {
        if((*getArtistStruct)->albums[i].id != NULL) { free((*getArtistStruct)->albums[i].id); }
        if((*getArtistStruct)->albums[i].parent != NULL) { free((*getArtistStruct)->albums[i].parent); }
        if((*getArtistStruct)->albums[i].title != NULL) { free((*getArtistStruct)->albums[i].title); }
        if((*getArtistStruct)->albums[i].name != NULL) { free((*getArtistStruct)->albums[i].name); }
        if((*getArtistStruct)->albums[i].coverArt != NULL) { free((*getArtistStruct)->albums[i].coverArt); }
        if((*getArtistStruct)->albums[i].created != NULL) { free((*getArtistStruct)->albums[i].created); }
        if((*getArtistStruct)->albums[i].artistId != NULL) { free((*getArtistStruct)->albums[i].artistId); }
        if((*getArtistStruct)->albums[i].artist != NULL) { free((*getArtistStruct)->albums[i].artist); }
        if((*getArtistStruct)->albums[i].genre != NULL) { free((*getArtistStruct)->albums[i].genre); }
        if((*getArtistStruct)->albums[i].starred != NULL) { free((*getArtistStruct)->albums[i].starred); }
    }
    if((*getArtistStruct)->albums != NULL) { free((*getArtistStruct)->albums); }
    if(*getArtistStruct != NULL) { free(*getArtistStruct); }
}

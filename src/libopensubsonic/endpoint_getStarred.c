#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getStarred.h"

// Returns 1 if failure occured, else 0
int opensubsonic_getStarred_parse(char* data, opensubsonic_getStarred_struct** getStarredStruct) {
    // Allocate and initialize
    *getStarredStruct = (opensubsonic_getStarred_struct*)malloc(sizeof(opensubsonic_getStarred_struct));
    (*getStarredStruct)->status = NULL;
    (*getStarredStruct)->errorMessage = NULL;
    (*getStarredStruct)->errorCode = 0;
    (*getStarredStruct)->artistCount = -1;
    (*getStarredStruct)->albumCount = -1;
    (*getStarredStruct)->songCount = -1;
    (*getStarredStruct)->artists = NULL;
    (*getStarredStruct)->albums = NULL;
    (*getStarredStruct)->songs = NULL;

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
    OSS_Psoj(&(*getStarredStruct)->status, subsonic_root, "status");
    if (strstr((*getStarredStruct)->status, "ok") == NULL) {
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getStarredStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getStarredStruct)->errorMessage, subsonic_error, "message");
        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getStarredStruct)->errorCode, (*getStarredStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from starred
    cJSON* starred_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "starred");
    if (starred_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - starred does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from artist, album, and song
    cJSON* artist_root = cJSON_GetObjectItemCaseSensitive(starred_root, "artist");
    if (artist_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - artist does not exist.");
    } else {
        (*getStarredStruct)->artistCount = 0;
    }

    cJSON* album_root = cJSON_GetObjectItemCaseSensitive(starred_root, "album");
    if (album_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - album does not exist.");
    } else {
        (*getStarredStruct)->albumCount = 0;
    }

    cJSON* song_root = cJSON_GetObjectItemCaseSensitive(starred_root, "song");
    if (song_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - song does not exist.");
    } else {
        (*getStarredStruct)->songCount = 0;
    }

    // Extract starred artists
    if ((*getStarredStruct)->artistCount != -1) {
        // Count, allocate, initialize, and extract
        (*getStarredStruct)->artistCount = cJSON_GetArraySize(artist_root);
        (*getStarredStruct)->artists = (opensubsonic_getStarred_artist_struct*)malloc((*getStarredStruct)->artistCount * sizeof(opensubsonic_getStarred_artist_struct));
        
        for (int i = 0; i < (*getStarredStruct)->artistCount; i++) {
            (*getStarredStruct)->artists[i].id = NULL;
            (*getStarredStruct)->artists[i].name = NULL;
            (*getStarredStruct)->artists[i].coverArt = NULL;
            (*getStarredStruct)->artists[i].starred = NULL;
        }
        
        for (int i = 0; i < (*getStarredStruct)->artistCount; i++) {
            cJSON* array_artist_root = cJSON_GetArrayItem(artist_root, i);
            if (array_artist_root != NULL) {
                OSS_Psoj(&(*getStarredStruct)->artists[i].id, array_artist_root, "id");
                OSS_Psoj(&(*getStarredStruct)->artists[i].name, array_artist_root, "name");
                OSS_Psoj(&(*getStarredStruct)->artists[i].coverArt, array_artist_root, "coverArt");
                OSS_Psoj(&(*getStarredStruct)->artists[i].starred, array_artist_root, "starred");
            }
        }
    } else {
        // No starred artists
        (*getStarredStruct)->artistCount = 0;
    }
    
    // Extract starred albums
    if ((*getStarredStruct)->albumCount != -1) {
        // Count, allocate, initialize, and extract
        (*getStarredStruct)->albumCount = cJSON_GetArraySize(album_root);
        (*getStarredStruct)->albums = (opensubsonic_getStarred_album_struct*)malloc((*getStarredStruct)->albumCount * sizeof(opensubsonic_getStarred_album_struct));

        for (int i = 0; i < (*getStarredStruct)->albumCount; i++) {
            (*getStarredStruct)->albums[i].id = NULL;
            (*getStarredStruct)->albums[i].parent = NULL;
            (*getStarredStruct)->albums[i].album = NULL;
            (*getStarredStruct)->albums[i].title = NULL;
            (*getStarredStruct)->albums[i].name = NULL;
            (*getStarredStruct)->albums[i].coverArt = NULL;
            (*getStarredStruct)->albums[i].songCount = 0;
            (*getStarredStruct)->albums[i].created = NULL;
            (*getStarredStruct)->albums[i].duration = 0;
            (*getStarredStruct)->albums[i].playCount = 0;
            (*getStarredStruct)->albums[i].artistId = NULL;
            (*getStarredStruct)->albums[i].artist = NULL;
            (*getStarredStruct)->albums[i].year = 0;
            (*getStarredStruct)->albums[i].genre = NULL;
        }
        
        for (int i = 0; i < (*getStarredStruct)->albumCount; i++) {
            cJSON* array_album_root = cJSON_GetArrayItem(album_root, i);
            if (array_album_root != NULL) {
                OSS_Psoj(&(*getStarredStruct)->albums[i].id, array_album_root, "id");
                OSS_Psoj(&(*getStarredStruct)->albums[i].parent, array_album_root, "parent");
                OSS_Psoj(&(*getStarredStruct)->albums[i].album, array_album_root, "album");
                OSS_Psoj(&(*getStarredStruct)->albums[i].title, array_album_root, "title");
                OSS_Psoj(&(*getStarredStruct)->albums[i].name, array_album_root, "name");
                OSS_Psoj(&(*getStarredStruct)->albums[i].coverArt, array_album_root, "coverArt");
                OSS_Pioj(&(*getStarredStruct)->albums[i].songCount, array_album_root, "songCount");
                OSS_Psoj(&(*getStarredStruct)->albums[i].created, array_album_root, "created");
                OSS_Ploj(&(*getStarredStruct)->albums[i].duration, array_album_root, "duration");
                OSS_Pioj(&(*getStarredStruct)->albums[i].playCount, array_album_root, "playCount");
                OSS_Psoj(&(*getStarredStruct)->albums[i].artistId, array_album_root, "artistId");
                OSS_Psoj(&(*getStarredStruct)->albums[i].artist, array_album_root, "artist");
                OSS_Pioj(&(*getStarredStruct)->albums[i].year, array_album_root, "year");
                OSS_Psoj(&(*getStarredStruct)->albums[i].genre, array_album_root, "genre");
            }
        }
    } else {
        // No starred albums
        (*getStarredStruct)->albumCount = 0;
    }
    
    // Extract starred songs
    if ((*getStarredStruct)->songCount != -1) {
        // Count, allocate, initialize, and extract
        (*getStarredStruct)->songCount = cJSON_GetArraySize(song_root);
        (*getStarredStruct)->songs = (opensubsonic_getStarred_song_struct*)malloc((*getStarredStruct)->songCount * sizeof(opensubsonic_getStarred_song_struct));
        
        for (int i = 0; i < (*getStarredStruct)->songCount; i++) {
            (*getStarredStruct)->songs[i].id = NULL;
            (*getStarredStruct)->songs[i].parent = NULL;
            (*getStarredStruct)->songs[i].title = NULL;
            (*getStarredStruct)->songs[i].album = NULL;
            (*getStarredStruct)->songs[i].artist = NULL;
            (*getStarredStruct)->songs[i].track = 0;
            (*getStarredStruct)->songs[i].year = 0;
            (*getStarredStruct)->songs[i].coverArt = NULL;
            (*getStarredStruct)->songs[i].size = 0;
            (*getStarredStruct)->songs[i].starred = NULL;
            (*getStarredStruct)->songs[i].duration = 0;
            (*getStarredStruct)->songs[i].bitRate = 0;
            (*getStarredStruct)->songs[i].bitDepth = 0;
            (*getStarredStruct)->songs[i].samplingRate = 0;
            (*getStarredStruct)->songs[i].channelCount = 0;
            (*getStarredStruct)->songs[i].playCount = 0;
            (*getStarredStruct)->songs[i].discNumber = 0;
            (*getStarredStruct)->songs[i].created = NULL;
            (*getStarredStruct)->songs[i].albumId = NULL;
            (*getStarredStruct)->songs[i].artistId = NULL;
        }
        
        for (int i = 0; i < (*getStarredStruct)->songCount; i++) {
            cJSON* array_song_root = cJSON_GetArrayItem(song_root, i);
            if (array_song_root != NULL) {
                OSS_Psoj(&(*getStarredStruct)->songs[i].id, array_song_root, "id");
                OSS_Psoj(&(*getStarredStruct)->songs[i].parent, array_song_root, "parent");
                OSS_Psoj(&(*getStarredStruct)->songs[i].title, array_song_root, "title");
                OSS_Psoj(&(*getStarredStruct)->songs[i].album, array_song_root, "album");
                OSS_Psoj(&(*getStarredStruct)->songs[i].artist, array_song_root, "artist");
                OSS_Pioj(&(*getStarredStruct)->songs[i].track, array_song_root, "track");
                OSS_Pioj(&(*getStarredStruct)->songs[i].year, array_song_root, "year");
                OSS_Psoj(&(*getStarredStruct)->songs[i].coverArt, array_song_root, "coverArt");
                OSS_Ploj(&(*getStarredStruct)->songs[i].size, array_song_root, "size");
                OSS_Psoj(&(*getStarredStruct)->songs[i].starred, array_song_root, "starred");
                OSS_Ploj(&(*getStarredStruct)->songs[i].duration, array_song_root, "duration");
                OSS_Pioj(&(*getStarredStruct)->songs[i].bitRate, array_song_root, "bitRate");
                OSS_Pioj(&(*getStarredStruct)->songs[i].bitDepth, array_song_root, "bitDepth");
                OSS_Ploj(&(*getStarredStruct)->songs[i].samplingRate, array_song_root, "samplingRate");
                OSS_Pioj(&(*getStarredStruct)->songs[i].channelCount, array_song_root, "channelCount");
                OSS_Pioj(&(*getStarredStruct)->songs[i].playCount, array_song_root, "playCount");
                OSS_Pioj(&(*getStarredStruct)->songs[i].discNumber, array_song_root, "discNumber");
                OSS_Psoj(&(*getStarredStruct)->songs[i].created, array_song_root, "created");
                OSS_Psoj(&(*getStarredStruct)->songs[i].albumId, array_song_root, "albumId");
                OSS_Psoj(&(*getStarredStruct)->songs[i].artistId, array_song_root, "artistId");
            }
        }
    } else {
        // No starred songs
        (*getStarredStruct)->songCount = 0;
    }

    cJSON_Delete(root);
    return 0;
}

void opensubsonic_getStarred_struct_free(opensubsonic_getStarred_struct** getStarredStruct) {
    logger_log_general(__func__, "Freeing /getStarred endpoint heap objects.");
    if ((*getStarredStruct)->status != NULL) { free((*getStarredStruct)->status); }
    if ((*getStarredStruct)->errorMessage != NULL) { free((*getStarredStruct)->errorMessage); }
    for (int i = 0; i < (*getStarredStruct)->artistCount; i++) {
        if ((*getStarredStruct)->artists[i].id != NULL) { free((*getStarredStruct)->artists[i].id); }
        if ((*getStarredStruct)->artists[i].name != NULL) { free((*getStarredStruct)->artists[i].name); }
        if ((*getStarredStruct)->artists[i].coverArt != NULL) { free((*getStarredStruct)->artists[i].coverArt); }
        if ((*getStarredStruct)->artists[i].starred != NULL) { free((*getStarredStruct)->artists[i].starred); }
    }
    for (int i = 0; i < (*getStarredStruct)->albumCount; i++) {
        if ((*getStarredStruct)->albums[i].id != NULL) { free((*getStarredStruct)->albums[i].id); }
        if ((*getStarredStruct)->albums[i].parent != NULL) { free((*getStarredStruct)->albums[i].parent); }
        if ((*getStarredStruct)->albums[i].album != NULL) { free((*getStarredStruct)->albums[i].album); }
        if ((*getStarredStruct)->albums[i].title != NULL) { free((*getStarredStruct)->albums[i].title); }
        if ((*getStarredStruct)->albums[i].name != NULL) { free((*getStarredStruct)->albums[i].name); }
        if ((*getStarredStruct)->albums[i].coverArt != NULL) { free((*getStarredStruct)->albums[i].coverArt); }
        if ((*getStarredStruct)->albums[i].created != NULL) { free((*getStarredStruct)->albums[i].created); }
        if ((*getStarredStruct)->albums[i].artistId != NULL) { free((*getStarredStruct)->albums[i].artistId); }
        if ((*getStarredStruct)->albums[i].artist != NULL) { free((*getStarredStruct)->albums[i].artist); }
        if ((*getStarredStruct)->albums[i].genre != NULL) { free((*getStarredStruct)->albums[i].genre); }
    }
    for (int i = 0; i < (*getStarredStruct)->songCount; i++) {
        if ((*getStarredStruct)->songs[i].id != NULL) { free((*getStarredStruct)->songs[i].id); }
        if ((*getStarredStruct)->songs[i].parent != NULL) { free((*getStarredStruct)->songs[i].parent); }
        if ((*getStarredStruct)->songs[i].title != NULL) { free((*getStarredStruct)->songs[i].title); }
        if ((*getStarredStruct)->songs[i].album != NULL) { free((*getStarredStruct)->songs[i].album); }
        if ((*getStarredStruct)->songs[i].artist != NULL) { free((*getStarredStruct)->songs[i].artist); }
        if ((*getStarredStruct)->songs[i].coverArt != NULL) { free((*getStarredStruct)->songs[i].coverArt); }
        if ((*getStarredStruct)->songs[i].starred != NULL) { free((*getStarredStruct)->songs[i].starred); }
        if ((*getStarredStruct)->songs[i].created != NULL) { free((*getStarredStruct)->songs[i].created); }
        if ((*getStarredStruct)->songs[i].albumId != NULL) { free((*getStarredStruct)->songs[i].albumId); }
        if ((*getStarredStruct)->songs[i].artistId != NULL) { free((*getStarredStruct)->songs[i].artistId); }
    }
    if ((*getStarredStruct)->artists != NULL) { free((*getStarredStruct)->artists); }
    if ((*getStarredStruct)->albums != NULL) { free((*getStarredStruct)->albums); }
    if ((*getStarredStruct)->songs != NULL) { free((*getStarredStruct)->songs); }
    if (*getStarredStruct != NULL) { free(*getStarredStruct); }
}

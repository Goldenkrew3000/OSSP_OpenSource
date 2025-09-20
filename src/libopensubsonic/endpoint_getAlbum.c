#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getAlbum.h"

// Returns 1 if failure occured, else 0
int opensubsonic_getAlbum_parse(char* data, opensubsonic_getAlbum_struct** getAlbumStruct) {
    // Allocate and initialize
    (*getAlbumStruct) = malloc(sizeof(opensubsonic_getAlbum_struct));
    (*getAlbumStruct)->status = NULL;
    (*getAlbumStruct)->errorCode = 0;
    (*getAlbumStruct)->errorMessage = NULL;
    (*getAlbumStruct)->id = NULL;
    (*getAlbumStruct)->parent = NULL;
    (*getAlbumStruct)->album = NULL;
    (*getAlbumStruct)->title = NULL;
    (*getAlbumStruct)->name = NULL;
    (*getAlbumStruct)->coverArt = NULL;
    (*getAlbumStruct)->created = NULL;
    (*getAlbumStruct)->duration = 0;
    (*getAlbumStruct)->playCount = 0;
    (*getAlbumStruct)->artistId = NULL;
    (*getAlbumStruct)->artist = NULL;
    (*getAlbumStruct)->year = 0;
    (*getAlbumStruct)->genre = NULL;
    (*getAlbumStruct)->songCount = 0;
    (*getAlbumStruct)->songs = NULL;

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
    OSS_Psoj(&(*getAlbumStruct)->status, subsonic_root, "status");
    if (strstr((*getAlbumStruct)->status, "ok") == NULL) {
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getAlbumStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getAlbumStruct)->errorMessage, subsonic_error, "message");
        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getAlbumStruct)->errorCode, (*getAlbumStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }

    // Process contents
    cJSON* album_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "album");
    if (album_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - album does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    OSS_Psoj(&(*getAlbumStruct)->id, album_root, "id");
    OSS_Psoj(&(*getAlbumStruct)->parent, album_root, "parent");
    OSS_Psoj(&(*getAlbumStruct)->album, album_root, "album");
    OSS_Psoj(&(*getAlbumStruct)->title, album_root, "title");
    OSS_Psoj(&(*getAlbumStruct)->name, album_root, "name");
    OSS_Psoj(&(*getAlbumStruct)->coverArt, album_root, "coverArt");
    OSS_Psoj(&(*getAlbumStruct)->created, album_root, "created");
    OSS_Ploj(&(*getAlbumStruct)->duration, album_root, "duration");
    OSS_Pioj(&(*getAlbumStruct)->playCount, album_root, "playCount");
    OSS_Psoj(&(*getAlbumStruct)->artistId, album_root, "artistId");
    OSS_Psoj(&(*getAlbumStruct)->artist, album_root, "artist");
    OSS_Pioj(&(*getAlbumStruct)->year, album_root, "year");
    OSS_Psoj(&(*getAlbumStruct)->genre, album_root, "genre");
    
    cJSON* song_root = cJSON_GetObjectItemCaseSensitive(album_root, "song");
    if (song_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - song does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    (*getAlbumStruct)->songCount = cJSON_GetArraySize(song_root);

    // Allocate and initialize
    (*getAlbumStruct)->songs = malloc((*getAlbumStruct)->songCount * sizeof(opensubsonic_getAlbum_song_struct));

    for (int i = 0; i < (*getAlbumStruct)->songCount; i++) {
        (*getAlbumStruct)->songs[i].id = NULL;
        (*getAlbumStruct)->songs[i].parent = NULL;
        (*getAlbumStruct)->songs[i].title = NULL;
        (*getAlbumStruct)->songs[i].albumId = NULL;
        (*getAlbumStruct)->songs[i].album = NULL;
        (*getAlbumStruct)->songs[i].artistId = NULL;
        (*getAlbumStruct)->songs[i].artist = NULL;
        (*getAlbumStruct)->songs[i].coverArt = NULL;
        (*getAlbumStruct)->songs[i].duration = 0;
        (*getAlbumStruct)->songs[i].bitRate = 0;
        (*getAlbumStruct)->songs[i].bitDepth = 0;
        (*getAlbumStruct)->songs[i].samplingRate = 0;
        (*getAlbumStruct)->songs[i].channelCount = 0;
        (*getAlbumStruct)->songs[i].track = 0;
        (*getAlbumStruct)->songs[i].year = 0;
        (*getAlbumStruct)->songs[i].genre = NULL;
        (*getAlbumStruct)->songs[i].size = 0;
        (*getAlbumStruct)->songs[i].discNumber = 0;
    }
    
    for (int i = 0; i < (*getAlbumStruct)->songCount; i++) {
        cJSON* array_song_root = cJSON_GetArrayItem(song_root, i);
        if (array_song_root != NULL) {
            OSS_Psoj(&(*getAlbumStruct)->songs[i].id, array_song_root, "id");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].parent, array_song_root, "parent");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].title, array_song_root, "title");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].albumId, array_song_root, "albumId");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].album, array_song_root, "album");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].artistId, array_song_root, "artistId");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].artist, array_song_root, "artist");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].coverArt, array_song_root, "coverArt");
            OSS_Ploj(&(*getAlbumStruct)->songs[i].duration, array_song_root, "duration");
            OSS_Pioj(&(*getAlbumStruct)->songs[i].bitRate, array_song_root, "bitRate");
            OSS_Pioj(&(*getAlbumStruct)->songs[i].bitDepth, array_song_root, "bitDepth");
            OSS_Ploj(&(*getAlbumStruct)->songs[i].samplingRate, array_song_root, "samplingRate");
            OSS_Pioj(&(*getAlbumStruct)->songs[i].channelCount, array_song_root, "channelCount");
            OSS_Pioj(&(*getAlbumStruct)->songs[i].track, array_song_root, "track");
            OSS_Pioj(&(*getAlbumStruct)->songs[i].year, array_song_root, "year");
            OSS_Psoj(&(*getAlbumStruct)->songs[i].genre, array_song_root, "genre");
            OSS_Ploj(&(*getAlbumStruct)->songs[i].size, array_song_root, "size");
            OSS_Pioj(&(*getAlbumStruct)->songs[i].discNumber, array_song_root, "discNumber");
        }
    }

    cJSON_Delete(root);
    return 0;
}

void opensubsonic_getAlbum_struct_free(opensubsonic_getAlbum_struct** getAlbumStruct) {
    logger_log_general(__func__, "Freeing /getAlbum endpoint heap objects.");
    if ((*getAlbumStruct)->status != NULL) { free((*getAlbumStruct)->status); }
    if ((*getAlbumStruct)->errorMessage != NULL) { free((*getAlbumStruct)->errorMessage); }
    if ((*getAlbumStruct)->id != NULL) { free((*getAlbumStruct)->id); }
    if ((*getAlbumStruct)->parent != NULL) { free((*getAlbumStruct)->parent); }
    if ((*getAlbumStruct)->album != NULL) { free((*getAlbumStruct)->album); }
    if ((*getAlbumStruct)->title != NULL) { free((*getAlbumStruct)->title); }
    if ((*getAlbumStruct)->name != NULL) { free((*getAlbumStruct)->name); }
    if ((*getAlbumStruct)->coverArt != NULL) { free((*getAlbumStruct)->coverArt); }
    if ((*getAlbumStruct)->created != NULL) { free((*getAlbumStruct)->created); }
    if ((*getAlbumStruct)->artistId != NULL) { free((*getAlbumStruct)->artistId); }
    if ((*getAlbumStruct)->artist != NULL) { free((*getAlbumStruct)->artist); }
    if ((*getAlbumStruct)->genre != NULL) { free((*getAlbumStruct)->genre); }
    for (int i = 0; i < (*getAlbumStruct)->songCount; i++) {
        if ((*getAlbumStruct)->songs[i].id != NULL) { free((*getAlbumStruct)->songs[i].id); }
        if ((*getAlbumStruct)->songs[i].parent != NULL) { free((*getAlbumStruct)->songs[i].parent); }
        if ((*getAlbumStruct)->songs[i].title != NULL) { free((*getAlbumStruct)->songs[i].title); }
        if ((*getAlbumStruct)->songs[i].albumId != NULL) { free((*getAlbumStruct)->songs[i].albumId); }
        if ((*getAlbumStruct)->songs[i].album != NULL) { free((*getAlbumStruct)->songs[i].album); }
        if ((*getAlbumStruct)->songs[i].artistId != NULL) { free((*getAlbumStruct)->songs[i].artistId); }
        if ((*getAlbumStruct)->songs[i].artist != NULL) { free((*getAlbumStruct)->songs[i].artist); }
        if ((*getAlbumStruct)->songs[i].coverArt != NULL) { free((*getAlbumStruct)->songs[i].coverArt); }
        if ((*getAlbumStruct)->songs[i].genre != NULL) { free((*getAlbumStruct)->songs[i].genre); }
    }
    if ((*getAlbumStruct)->songs != NULL) { free((*getAlbumStruct)->songs); }
    if (*getAlbumStruct != NULL) { free(*getAlbumStruct); }
}

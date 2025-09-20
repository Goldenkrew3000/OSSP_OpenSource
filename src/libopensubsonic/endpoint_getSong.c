#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getSong.h"

// Parse the JSON returned from the /rest/getSong endpoint
// Returns 1 if failure occured, else 0
int opensubsonic_getSong_parse(char* data, opensubsonic_getSong_struct** getSongStruct) {
    // Allocate struct
    *getSongStruct = malloc(sizeof(opensubsonic_getSong_struct));
    
    // Initialize struct variables
    (*getSongStruct)->status = NULL;
    (*getSongStruct)->errorCode = 0;
    (*getSongStruct)->errorMessage = NULL;
    (*getSongStruct)->id = NULL;
    (*getSongStruct)->parent = NULL;
    (*getSongStruct)->title = NULL;
    (*getSongStruct)->album = NULL;
    (*getSongStruct)->artist = NULL;
    (*getSongStruct)->track = 0;
    (*getSongStruct)->year = 0;
    (*getSongStruct)->coverArt = NULL;
    (*getSongStruct)->size = 0;
    (*getSongStruct)->starred = NULL;
    (*getSongStruct)->duration = 0;
    (*getSongStruct)->bitRate = 0;
    (*getSongStruct)->bitDepth = 0;
    (*getSongStruct)->samplingRate = 0;
    (*getSongStruct)->channelCount = 0;
    (*getSongStruct)->played = NULL;
    (*getSongStruct)->discNumber = 0;
    (*getSongStruct)->created = NULL;
    (*getSongStruct)->albumId = NULL;
    (*getSongStruct)->artistId = NULL;
    (*getSongStruct)->displayArtist = NULL;
    (*getSongStruct)->displayAlbumArtist = NULL;
    (*getSongStruct)->displayComposer = NULL;
    
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
    
    OSS_Psoj(&(*getSongStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getSongStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getSongStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getSongStruct)->errorMessage, subsonic_error, "message");

        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getSongStruct)->errorCode, (*getSongStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }
    
    // Make an object from song
    cJSON* song_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "song");
    if (song_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - song does not exist.");
        cJSON_Delete(root);
        return 1;
    }
    
    // Fetch song information
    OSS_Psoj(&(*getSongStruct)->id, song_root, "id");
    OSS_Psoj(&(*getSongStruct)->parent, song_root, "parent");
    OSS_Psoj(&(*getSongStruct)->title, song_root, "title");
    OSS_Psoj(&(*getSongStruct)->album, song_root, "album");
    OSS_Psoj(&(*getSongStruct)->artist, song_root, "artist");
    OSS_Pioj(&(*getSongStruct)->track, song_root, "track");
    OSS_Pioj(&(*getSongStruct)->year, song_root, "year");
    OSS_Psoj(&(*getSongStruct)->coverArt, song_root, "coverArt");
    OSS_Ploj(&(*getSongStruct)->size, song_root, "size");
    OSS_Psoj(&(*getSongStruct)->starred, song_root, "starred");
    OSS_Ploj(&(*getSongStruct)->duration, song_root, "duration");
    OSS_Pioj(&(*getSongStruct)->bitRate, song_root, "bitRate");
    OSS_Pioj(&(*getSongStruct)->bitDepth, song_root, "bitDepth");
    OSS_Ploj(&(*getSongStruct)->samplingRate, song_root, "samplingRate");
    OSS_Pioj(&(*getSongStruct)->channelCount, song_root, "channelCount");
    OSS_Psoj(&(*getSongStruct)->played, song_root, "played");
    OSS_Pioj(&(*getSongStruct)->discNumber, song_root, "discNumber");
    OSS_Psoj(&(*getSongStruct)->created, song_root, "created");
    OSS_Psoj(&(*getSongStruct)->albumId, song_root, "albumId");
    OSS_Psoj(&(*getSongStruct)->artistId, song_root, "artistId");
    OSS_Psoj(&(*getSongStruct)->displayArtist, song_root, "displayArtist");
    OSS_Psoj(&(*getSongStruct)->displayAlbumArtist, song_root, "displayAlbumArtist");
    OSS_Psoj(&(*getSongStruct)->displayComposer, song_root, "displayComposer");
    
    cJSON_Delete(root);
    return 0;
}

// Free the dynamically allocated elements of the opensubsonic_getSong_struct structure
void opensubsonic_getSong_struct_free(opensubsonic_getSong_struct** getSongStruct) {
    logger_log_general(__func__, "Freeing /getSong endpoint heap objects.");
    if ((*getSongStruct)->status != NULL) { free((*getSongStruct)->status); }
    if ((*getSongStruct)->errorMessage != NULL) { free((*getSongStruct)->errorMessage); }
    if ((*getSongStruct)->id != NULL) { free((*getSongStruct)->id); }
    if ((*getSongStruct)->parent != NULL) { free((*getSongStruct)->parent); }
    if ((*getSongStruct)->title != NULL) { free((*getSongStruct)->title); }
    if ((*getSongStruct)->album != NULL) { free((*getSongStruct)->album); }
    if ((*getSongStruct)->artist != NULL) { free((*getSongStruct)->artist); }
    if ((*getSongStruct)->coverArt != NULL) { free((*getSongStruct)->coverArt); }
    if ((*getSongStruct)->starred != NULL) { free((*getSongStruct)->starred); }
    if ((*getSongStruct)->played != NULL) { free((*getSongStruct)->played); }
    if ((*getSongStruct)->created != NULL) { free((*getSongStruct)->created); }
    if ((*getSongStruct)->albumId != NULL) { free((*getSongStruct)->albumId); }
    if ((*getSongStruct)->artistId != NULL) { free((*getSongStruct)->artistId); }
    if ((*getSongStruct)->displayArtist != NULL) { free((*getSongStruct)->displayArtist); }
    if ((*getSongStruct)->displayAlbumArtist != NULL) { free((*getSongStruct)->displayAlbumArtist); }
    if ((*getSongStruct)->displayComposer != NULL) { free((*getSongStruct)->displayComposer); }
    if (*getSongStruct != NULL) { free(*getSongStruct); }
}

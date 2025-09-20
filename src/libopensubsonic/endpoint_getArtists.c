#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"
#include "endpoint_getArtists.h"

// Parse the JSON returned from the /rest/getArtists endpoint
// Returns 1 if failure occured, else 0
int opensubsonic_getArtists_parse(char* data, opensubsonic_getArtists_struct** getArtistsStruct) {
    // Allocate struct
    *getArtistsStruct = (opensubsonic_getArtists_struct*)malloc(sizeof(opensubsonic_getArtists_struct));
    
    // Initialize struct variables
    (*getArtistsStruct)->status = NULL;
    (*getArtistsStruct)->errorMessage = NULL;
    (*getArtistsStruct)->errorCode = 0;
    (*getArtistsStruct)->artistCount = 0;
    (*getArtistsStruct)->artists = NULL;

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

    OSS_Psoj(&(*getArtistsStruct)->status, subsonic_root, "status");

    // Check if API has returned an error
    if (strstr((*getArtistsStruct)->status, "ok") == NULL) {
        // API has not returned 'ok' in status, fetch error, and return
        // Check if an error is present
        cJSON* subsonic_error = cJSON_GetObjectItemCaseSensitive(subsonic_root, "error");
        if (subsonic_error == NULL) {
            // Error not defined in JSON
            logger_log_error(__func__, "API has indicated failure through status, but error does not exist.");
            cJSON_Delete(root);
            return 1;
        }

        OSS_Pioj(&(*getArtistsStruct)->errorCode, subsonic_error, "code");
        OSS_Psoj(&(*getArtistsStruct)->errorMessage, subsonic_error, "message");
        
        logger_log_error(__func__, "Error noted in JSON - Code %d: %s", (*getArtistsStruct)->errorCode, (*getArtistsStruct)->errorMessage);
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from artists
    cJSON* artists_root = cJSON_GetObjectItemCaseSensitive(subsonic_root, "artists");
    if (artists_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - artists does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from index
    cJSON* index_root = cJSON_GetObjectItemCaseSensitive(artists_root, "index");
    if (index_root == NULL) {
        logger_log_error(__func__, "Error handling JSON - index does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Count the amount of artists present for malloc (Copied from function below)
    int totalArtistIndex = 0;
    for (int i = 0; i < cJSON_GetArraySize(index_root); i++) {
        cJSON* array_letter_root = cJSON_GetArrayItem(index_root, i);

        cJSON* current_letter_artists_array = cJSON_GetObjectItemCaseSensitive(array_letter_root, "artist");
        if (current_letter_artists_array != NULL) {
            // Array of artists starting with the same letter
            for (int j = 0; j < cJSON_GetArraySize(current_letter_artists_array); j++) {
                cJSON* current_letter_artist_array_layer_b = cJSON_GetArrayItem(current_letter_artists_array, j);
                if (current_letter_artist_array_layer_b != NULL) {
                    // Increment total artist index
                    (*getArtistsStruct)->artistCount++;
                }
            }
        }
    }

    // Allocate memory for opensubsonic_getArtists_artist_struct inside opensubsonic_getArtists_struct (Where the artist data is held)
    (*getArtistsStruct)->artists = (opensubsonic_getArtists_artist_struct*)malloc((*getArtistsStruct)->artistCount * sizeof(opensubsonic_getArtists_artist_struct));

    // Initialize struct variables
    for (int i = 0; i < (*getArtistsStruct)->artistCount; i++) {
        (*getArtistsStruct)->artists[i].name = NULL;
        (*getArtistsStruct)->artists[i].id = NULL;
        (*getArtistsStruct)->artists[i].coverArt = NULL;
        (*getArtistsStruct)->artists[i].albumCount = 0;
    }
    
    // Go through the alphabet array (Each element in this array is the first letter of the artist, organized)
    int currentArtistIndex = 0;
    for (int i = 0; i < cJSON_GetArraySize(index_root); i++) {
        cJSON* array_letter_root = cJSON_GetArrayItem(index_root, i);

        cJSON* current_letter_artists_array = cJSON_GetObjectItemCaseSensitive(array_letter_root, "artist");
        if (current_letter_artists_array != NULL) {
            // Array of artists starting with the same letter
            for (int j = 0; j < cJSON_GetArraySize(current_letter_artists_array); j++) {
                cJSON* current_letter_artist_array_layer_b = cJSON_GetArrayItem(current_letter_artists_array, j);
                if (current_letter_artist_array_layer_b != NULL) {
                    // Fetch information
                    OSS_Psoj(&(*getArtistsStruct)->artists[currentArtistIndex].name, current_letter_artist_array_layer_b, "name");
                    OSS_Psoj(&(*getArtistsStruct)->artists[currentArtistIndex].id, current_letter_artist_array_layer_b, "id");
                    OSS_Psoj(&(*getArtistsStruct)->artists[currentArtistIndex].coverArt, current_letter_artist_array_layer_b, "coverArt");
                    OSS_Pioj(&(*getArtistsStruct)->artists[currentArtistIndex].albumCount, current_letter_artist_array_layer_b, "albumCount");

                    // Increment current artist index
                    currentArtistIndex++;
                }
            }
        }
    }

    cJSON_Delete(root);
    return 0;
}

// Free the opensubsonic_getArtists_struct structure.
void opensubsonic_getArtists_struct_free(opensubsonic_getArtists_struct** getArtistsStruct) {
    logger_log_general(__func__, "Freeing /getArtists endpoint heap objects.");
    if ((*getArtistsStruct)->status != NULL) { free((*getArtistsStruct)->status); }
    if ((*getArtistsStruct)->errorMessage != NULL) { free((*getArtistsStruct)->errorMessage); }
    for (size_t i = 0; i < (*getArtistsStruct)->artistCount; i++) {
        if ((*getArtistsStruct)->artists[i].name != NULL) { free((*getArtistsStruct)->artists[i].name); }
        if ((*getArtistsStruct)->artists[i].id != NULL) { free((*getArtistsStruct)->artists[i].id); }
        if ((*getArtistsStruct)->artists[i].coverArt != NULL) { free((*getArtistsStruct)->artists[i].coverArt); }
    }
    if ((*getArtistsStruct)->artists != NULL) { free((*getArtistsStruct)->artists); }
    if (*getArtistsStruct != NULL) { free(*getArtistsStruct); }
}

/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Socket Actions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketActions.h"
#include "../player/player.h"
#include "../player/playQueue.hpp"
#include "../libopensubsonic/utils.h"
#include "../libopensubsonic/httpclient.h"
#include "../libopensubsonic/endpoint_getStarred.h"
#include "../libopensubsonic/endpoint_getSong.h"
#include "../external/cJSON.h"

void OSSPS_SocketAction_Get_Starred_Songs(char** retDataStr, cJSON** cliReqJson) {
    // Fetch /getStarred endpoint
    opensubsonic_httpClient_URL_t* starredUrl = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&starredUrl);
    starredUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETSTARRED;
    opensubsonic_httpClient_formUrl(&starredUrl);
    opensubsonic_getStarred_struct* getStarredStruct;
    opensubsonic_httpClient_fetchResponse(&starredUrl, (void**)&getStarredStruct);
    opensubsonic_httpClient_URL_cleanup(&starredUrl);

    // Filter out the starred songs only
    cJSON* retPayload = cJSON_CreateObject();
    cJSON_AddItemToObject(retPayload, "songCount", cJSON_CreateNumber(getStarredStruct->songCount));
            
    cJSON* songArray = cJSON_CreateArray();
    cJSON_AddItemToObject(retPayload, "songs", songArray);

    for (int i = 0; i < getStarredStruct->songCount; i++) {
        // NOTE: For anything the client isn't directly accessing, only pass IDs
        // Client uses cover art directly, so it needs a URL, but it only needs to pass a song ID back
        opensubsonic_httpClient_URL_t* coverArtUrl = malloc(sizeof(opensubsonic_httpClient_URL_t));
        opensubsonic_httpClient_URL_prepare(&coverArtUrl);
        coverArtUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
        coverArtUrl->id = strdup(getStarredStruct->songs[i].coverArt);
        opensubsonic_httpClient_formUrl(&coverArtUrl);

        cJSON* songObj = cJSON_CreateObject();
        cJSON_AddItemToObject(songObj, "title", cJSON_CreateString(getStarredStruct->songs[i].title));
        cJSON_AddItemToObject(songObj, "album", cJSON_CreateString(getStarredStruct->songs[i].album));
        cJSON_AddItemToObject(songObj, "artist", cJSON_CreateString(getStarredStruct->songs[i].artist));
        cJSON_AddItemToObject(songObj, "id", cJSON_CreateString(getStarredStruct->songs[i].id));
        cJSON_AddItemToObject(songObj, "size", cJSON_CreateNumber(getStarredStruct->songs[i].size));
        cJSON_AddItemToObject(songObj, "duration", cJSON_CreateNumber(getStarredStruct->songs[i].duration));
        cJSON_AddItemToObject(songObj, "albumId", cJSON_CreateString(getStarredStruct->songs[i].albumId));
        cJSON_AddItemToObject(songObj, "artistId", cJSON_CreateString(getStarredStruct->songs[i].artistId));
        cJSON_AddItemToObject(songObj, "coverArtUrl", cJSON_CreateString(coverArtUrl->formedUrl));
        cJSON_AddItemToArray(songArray, songObj);

        opensubsonic_httpClient_URL_cleanup(&coverArtUrl);
    }

    // Create return payload string and free memory
    opensubsonic_getStarred_struct_free(&getStarredStruct);
    *retDataStr = cJSON_PrintUnformatted(retPayload);
    cJSON_Delete(retPayload);
    return;
}

void OSSPS_SocketAction_Now_Playing(char** retDataStr, cJSON** cliReqJson) {
    cJSON* retPayload = cJSON_CreateObject();
    int currentPos = OSSPQ_getCurrentPos();
    if (currentPos == 0) {
        // No songs added to queue yet
        cJSON_AddItemToObject(retPayload, "totalQueueCount", cJSON_CreateNumber(0));
    } else {
        // At least a single song has been added to the queue
        OSSPQ_SongStruct* nowPlaying = OSSPQ_getAtPos(currentPos);
        if (nowPlaying == NULL) {
            // Could not pull queue item
            // TODO
            printf("[SocketHandler] --\n");
        }

        cJSON_AddItemToObject(retPayload, "songTitle", cJSON_CreateString(nowPlaying->title));
        cJSON_AddItemToObject(retPayload, "songAlbum", cJSON_CreateString(nowPlaying->album));
        cJSON_AddItemToObject(retPayload, "songArtist", cJSON_CreateString(nowPlaying->artist));
        //cJSON_AddItemToObject(retData, "duration", cJSON_CreateString());
        cJSON_AddItemToObject(retPayload, "coverArtUrl", cJSON_CreateString(nowPlaying->coverArtUrl));
    }

    *retDataStr = cJSON_PrintUnformatted(retPayload);
    cJSON_Delete(retPayload);
    return;
}

void OSSPS_SocketAction_Add_To_Queue(char** retDataStr, cJSON** cliReqJson) {
    // Pull ID from request JSON
    char* id = NULL;
    OSS_Psoj(&id, *cliReqJson, "songId");
    if (id == NULL) {
        printf("[SocketHandler] OSSP_SOCKET_ACTION_ADD_TO_QUEUE failed - 'id' is null.\n");
        *retDataStr = strdup("NOTOK");
        return;
    }

    // Create Stream URL from ID
    opensubsonic_httpClient_URL_t* streamUrl = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&streamUrl);
    streamUrl->endpoint = OPENSUBSONIC_ENDPOINT_STREAM;
    streamUrl->id = strdup(id);
    opensubsonic_httpClient_formUrl(&streamUrl);

    // Contact the /getSong endpoint
    opensubsonic_httpClient_URL_t* songUrl = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&songUrl);
    songUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETSONG;
    songUrl->id = strdup(id);
    opensubsonic_httpClient_formUrl(&songUrl);
    opensubsonic_getSong_struct* getSongStruct;
    opensubsonic_httpClient_fetchResponse(&songUrl, (void**)&getSongStruct);

    // Create Cover Art URL from ID
    opensubsonic_httpClient_URL_t* coverartUrl = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&coverartUrl);
    coverartUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
    coverartUrl->id = strdup(id);
    opensubsonic_httpClient_formUrl(&coverartUrl);

    // Append to queue
    OSSPQ_AppendToEnd(getSongStruct->title,
                      getSongStruct->album,
                      getSongStruct->artist,
                      id,
                      streamUrl->formedUrl,
                      coverartUrl->formedUrl,
                      getSongStruct->duration,
                      OSSPQ_MODE_OPENSUBSONIC);

    // Free memory
    opensubsonic_getSong_struct_free(&getSongStruct);
    opensubsonic_httpClient_URL_cleanup(&songUrl);
    opensubsonic_httpClient_URL_cleanup(&streamUrl);
    opensubsonic_httpClient_URL_cleanup(&coverartUrl);

    *retDataStr = strdup("OK");
    return;
}

// NOTE: Could refactor OSSPS_SocketAction_OSSPP_OutVolume and OSSPS_SocketAction_OSSPP_InVolume into a single function,
// but at this point, I don't think it's worth it. This might change later
void OSSPS_SocketAction_OSSPP_OutVolume(char** retDataStr, cJSON** cliReqJson) {
    int vol = 0;
    OSS_Pioj(&vol, *cliReqJson, "vol");
    float f_vol = (float)vol / 100.0f;
    OSSPlayer_GstECont_OutVolume_set(f_vol);
    *retDataStr = strdup("OK");
}

void OSSPS_SocketAction_OSSPP_InVolume(char** retDataStr, cJSON** cliReqJson) {
    int vol = 0;
    OSS_Pioj(&vol, *cliReqJson, "vol");
    float f_vol = (float)vol / 100.0f;
    OSSPlayer_GstECont_InVolume_set(f_vol);
    *retDataStr = strdup("OK");
}

#include "OSSP_Bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include "libopensubsonic/crypto.h"
#include "libopensubsonic/httpclient.h"
#include "libopensubsonic/endpoint_ping.h"
#include "libopensubsonic/endpoint_getStarred.h"
#include "libopensubsonic/endpoint_getAlbum.h"

#include "libopensubsonic/endpoint_getPlaylists.h"
#include "libopensubsonic/endpoint_getPlaylist.h"
#include "libopensubsonic/endpoint_getSong.h"
#include "libopensubsonic/endpoint_getLyricsBySongId.h"
#include "libopensubsonic/endpoint_getAlbumList.h"
#include "libopensubsonic/endpoint_scrobble.h"
#include "configHandler.h"

#include "libopensubsonic/scrobble_lastFm.h"
#include "dscrdrpc.h"



struct memory {
    char *data;
    size_t size;
};

static size_t write_to_memory(void *ptr, size_t size, size_t nmemb, void *userdata) {
    struct memory *mem = (struct memory *)userdata;
    size_t total_size = size * nmemb;

    mem->data = realloc(mem->data, mem->size + total_size + 1);
    if (!mem->data) return 0;  // Fail on OOM

    memcpy(&(mem->data[mem->size]), ptr, total_size);
    mem->size += total_size;
    mem->data[mem->size] = '\0';  // Null-terminate

    return total_size;
}


static int rc = 0;
configHandler_config_t* configObj = NULL;

opensubsonic_ping_struct* LOSS_PerformLogin(C_LoginStruct loginStruct) {
    // Initialize configuration
    configHandler_Read(&configObj);
    
    // Generate the login token and salt
    opensubsonic_crypto_generateLogin();
    
    
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_PING;
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_ping_struct* OSS_ping_struct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&OSS_ping_struct);
    
    // Print Response
    //printf("Result: %s\n", OSS_ping_struct.status);
    
    //opensubsonic_ping_struct_free(&OSS_ping_struct);
    opensubsonic_httpClient_URL_cleanup(&url);
    
    //configHandler_Free(&config);
    //opensubsonic_authenticate_lastFm();
    
    
    
    /*
    opensubsonic_httpClient_URL_t* urlc = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&urlc);
    urlc->endpoint = OPENSUBSONIC_ENDPOINT_GETLYRICSBYSONGID;
    urlc->id = strdup("pvu323svJrim683Xf8fBV8");
    printf("URLC Endponit: %d\n", urlc->endpoint);
    opensubsonic_httpClient_formUrl(&urlc);
    printf("URL: %s\n", urlc->formedUrl);
    
    opensubsonic_getLyricsBySongId_struct* plStruct;
    opensubsonic_httpClient_fetchResponse(&urlc, (void**)&plStruct);
    
    printf("2nd: %s\n", plStruct->lyrics[1].data);
     */
    
    
    
    
    
    
    
    return OSS_ping_struct; // TODO rename
}


void LOSS_FreeLoginStruct() {
    
}


opensubsonic_getStarred_struct* LOSS_getStarred(void) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETSTARRED;
    opensubsonic_httpClient_formUrl(&url);
    printf("STARRED URL: %s\n", url->formedUrl);
    
    opensubsonic_getStarred_struct* getStarredStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getStarredStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
    
    return getStarredStruct;
}

void LOSS_delete_getStarred(opensubsonic_getStarred_struct* starredStruct) {
    opensubsonic_getStarred_struct_free(&starredStruct);
}

opensubsonic_getSong_struct* LOSS_getSong(char* id) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETSONG;
    url->id = strdup(id);
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getSong_struct* getSongStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getSongStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
    
    return getSongStruct;
}

void LOSS_delete_getSong(opensubsonic_getSong_struct* songStruct) {
    opensubsonic_getSong_struct_free(&songStruct);
}






opensubsonic_getAlbum_struct* LOSS_getAlbum(char* id) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETALBUM;
    url->id = strdup(id);
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getAlbum_struct* alStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&alStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
    
    return alStruct;
}

void LOSS_deleteAlbum(opensubsonic_getAlbum_struct* albumStruct) {
    //opensubsonic_getAlbum_struct_free(albumStruct); // TODO
}



// no error checking right now because fuck that TODO
opensubsonic_getAlbumList_struct* LOSS_getAlbumList(int type, int amount) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETALBUMLIST;
    url->type = type;
    url->amount = amount;
    opensubsonic_httpClient_formUrl(&url);
    printf("%s\n", url->formedUrl);
    
    opensubsonic_getAlbumList_struct* alStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&alStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
    
    return alStruct;
}

opensubsonic_getPlaylists_struct* LOSS_getPlaylists(void) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETPLAYLISTS;
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getPlaylists_struct* alStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&alStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
    
    return alStruct;
}





char* LOSS_getCoverArt(char* id) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
    url->id = strdup(id);
    opensubsonic_httpClient_formUrl(&url);
    
    return url->formedUrl;
    // TODO need to free URL struct
}


char* LOSS_Stream(char* id) {
    // Perform scrobble to navidrome (Used purely for server side tracking)
    opensubsonic_httpClient_URL_t* scrobble_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&scrobble_url);
    scrobble_url->endpoint = OPENSUBSONIC_ENDPOINT_SCROBBLE;
    scrobble_url->id = strdup(id);
    scrobble_url->submit = true;
    opensubsonic_httpClient_formUrl(&scrobble_url);
    opensubsonic_scrobble_struct* scrobbleStruct;
    opensubsonic_httpClient_fetchResponse(&scrobble_url, (void**)&scrobbleStruct);
    opensubsonic_scrobble_struct_free(&scrobbleStruct);
    opensubsonic_httpClient_URL_cleanup(&scrobble_url);
    
    // Actually make the stream url
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_STREAM;
    url->id = strdup(id);
    opensubsonic_httpClient_formUrl(&url);
    
    return url->formedUrl;
    // TODO need to free URL struct
}

void discordrpc_update(int type, opensubsonic_getSong_struct* songStruct) {
    printf("Updating Discord RPC\n");
    
    dscrdrpc_data* dscrdrpc = NULL;
    dscrdrpc_struct_init(&dscrdrpc);
    
    if (type == 1) {
        dscrdrpc->requestType = DSCRDRPC_REQTYPE_PLAYING;
    } else if (type == 2) {
        dscrdrpc->requestType = DSCRDRPC_REQTYPE_KEEPALIVE;
    } else if (type == 3) {
        dscrdrpc->requestType = DSCRDRPC_REQTYPE_PAUSED;
    }
    dscrdrpc->songTitle = strdup(songStruct->title);
    dscrdrpc->songArtist = strdup(songStruct->artist);
    dscrdrpc->coverArtUrl = LOSS_getCoverArt(songStruct->coverArt);
    dscrdrpc->deviceInfo = strdup("iPhone (iOS 16.6)");
    dscrdrpc->songLength = songStruct->duration;
    
    dscrdrpc_encrypt(&dscrdrpc);
    printf("Checksum: %s\n", dscrdrpc->checksum);
    
    dscrdrpc_struct_deinit(&dscrdrpc);
}

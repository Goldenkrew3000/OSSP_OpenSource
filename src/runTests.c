#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Other imports
#include "configHandler.h"
#include "libopensubsonic/logger.h"
#include "libopensubsonic/httpclient.h"
#include "libopensubsonic/crypto.h"

// libopensubsonic imports
#include "libopensubsonic/endpoint_ping.h"
#include "libopensubsonic/endpoint_getStarred.h"
#include "libopensubsonic/endpoint_getPlaylists.h"
#include "libopensubsonic/endpoint_getPlaylist.h"
#include "libopensubsonic/endpoint_getSong.h"
#include "libopensubsonic/endpoint_getArtists.h"
#include "libopensubsonic/endpoint_getArtist.h"
#include "libopensubsonic/endpoint_getAlbumList.h"
#include "libopensubsonic/endpoint_getAlbum.h"
#include "libopensubsonic/endpoint_getLyricsBySongId.h"

configHandler_config_t* configObj = NULL;

void test_libopensubsonic_endpoint_ping(void) {
    logger_log_general(__func__, "Testing ping endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_PING;
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_ping_struct* OSS_ping_struct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&OSS_ping_struct);
    
    opensubsonic_ping_struct_free(&OSS_ping_struct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getStarred(void) {
    logger_log_general(__func__, "Testing getStarred endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETSTARRED;
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getStarred_struct* getStarredStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getStarredStruct);
    
    opensubsonic_getStarred_struct_free(&getStarredStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getPlaylists(void) {
    logger_log_general(__func__, "Testing getPlaylists endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETPLAYLISTS;
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getPlaylists_struct* getPlaylistsStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getPlaylistsStruct);
    
    opensubsonic_getPlaylists_struct_free(&getPlaylistsStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getPlaylist(void) {
    logger_log_general(__func__, "Testing getPlaylist endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETPLAYLIST;
    url->id = strdup("dkBA3oVi5zpChEVIDVJn4i");
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getPlaylist_struct* getPlaylistStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getPlaylistStruct);
    
    opensubsonic_getPlaylist_struct_free(&getPlaylistStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getSong(void) {
    logger_log_general(__func__, "Testing getSong endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETSONG;
    url->id = strdup("c24SgyyHpe86IsAwSV73rx");
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getSong_struct* getSongStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getSongStruct);
    
    opensubsonic_getSong_struct_free(&getSongStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getArtists(void) {
    logger_log_general(__func__, "Testing getArtists endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETARTISTS;
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getArtists_struct* getArtistsStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getArtistsStruct);
    
    opensubsonic_getArtists_struct_free(&getArtistsStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getArtist(void) {
    logger_log_general(__func__, "Testing getArtist endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETARTIST;
    url->id = strdup("3mZKW6zlodlLW4x4QLDhPr");
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getArtist_struct* getArtistStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getArtistStruct);
    
    opensubsonic_getArtist_struct_free(&getArtistStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getAlbumList(void) {
    logger_log_general(__func__, "Testing getAlbumList endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETALBUMLIST;
    url->type = OPENSUBSONIC_ENDPOINT_GETALBUMLIST_RECENT;
    url->amount = 5;
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getAlbumList_struct* getAlbumListStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getAlbumListStruct);
    
    opensubsonic_getAlbumList_struct_free(&getAlbumListStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getAlbum(void) {
    logger_log_general(__func__, "Testing getAlbum endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETALBUM;
    url->id = strdup("5OjqcCVp8LbDfamfubZxFN");
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getAlbum_struct* getAlbumStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getAlbumStruct);
    
    opensubsonic_getAlbum_struct_free(&getAlbumStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

void test_libopensubsonic_endpoint_getLyricsBySongId(void) {
    logger_log_general(__func__, "Testing getLyricsBySongId endpoint.");
    
    opensubsonic_httpClient_URL_t* url = malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETLYRICSBYSONGID;
    url->id = strdup("pvu323svJrim683Xf8fBV8");
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_getLyricsBySongId_struct* getLyricsBySongIdStruct;
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getLyricsBySongIdStruct);
    
    opensubsonic_getLyricsBySongId_struct_free(&getLyricsBySongIdStruct);
    opensubsonic_httpClient_URL_cleanup(&url);
}

int main(void) {
    int rc = 0;

    // Read config file
    rc = configHandler_Read(&configObj);
    if (rc != 0) {
        printf("Could not read config file.\n");
        return 1;
    }
    opensubsonic_crypto_generateLogin();
    
    // Run tests
    test_libopensubsonic_endpoint_ping();
    test_libopensubsonic_endpoint_getStarred();
    test_libopensubsonic_endpoint_getPlaylists();
    test_libopensubsonic_endpoint_getPlaylist();
    test_libopensubsonic_endpoint_getSong();
    test_libopensubsonic_endpoint_getArtists();
    test_libopensubsonic_endpoint_getAlbumList();
    test_libopensubsonic_endpoint_getAlbum();
    test_libopensubsonic_endpoint_getLyricsBySongId();
    
    // Free config file
    configHandler_Free(&configObj);
}


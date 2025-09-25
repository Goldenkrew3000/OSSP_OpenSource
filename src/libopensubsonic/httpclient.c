#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../external/cJSON.h"
#include "httpclient.h"
#include "logger.h"
#include "../configHandler.h"
#include "logger.h"

#include "endpoint_ping.h"
#include "endpoint_getStarred.h"
#include "endpoint_getSong.h"
#include "endpoint_getPlaylists.h"
#include "endpoint_getPlaylist.h"
#include "endpoint_getArtists.h"
#include "endpoint_getArtist.h"
#include "endpoint_getLyricsBySongId.h"
#include "endpoint_getAlbumList.h"
#include "endpoint_getAlbum.h"
#include "endpoint_scrobble.h"

static int rc = 0;
extern configHandler_config_t* configObj;

void opensubsonic_httpClient_URL_prepare(opensubsonic_httpClient_URL_t** urlObj) {
    // Initialize struct variables
    (*urlObj)->endpoint = 0;
    (*urlObj)->id = NULL;
    (*urlObj)->type = 0;
    (*urlObj)->amount = 0;
    (*urlObj)->submit = false;
    (*urlObj)->formedUrl = NULL;
}

void opensubsonic_httpClient_URL_cleanup(opensubsonic_httpClient_URL_t** urlObj) {
    if ((*urlObj)->formedUrl != NULL) { free((*urlObj)->formedUrl); }
    if ((*urlObj)->id != NULL) { free((*urlObj)->id); }
    if (*urlObj != NULL) { free(*urlObj); }
}

void opensubsonic_httpClient_formUrl(opensubsonic_httpClient_URL_t** urlObj) {
    // TODO fix hack, add error checking,
    
    char* url = NULL;
    
    switch ((*urlObj)->endpoint) {
        case OPENSUBSONIC_ENDPOINT_PING:
            rc = asprintf(&url, "%s://%s/rest/ping?u=%s&t=%s&s=%s&f=json&v=%s&c=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName);
            break;
        case OPENSUBSONIC_ENDPOINT_GETSTARRED:
            rc = asprintf(&url, "%s://%s/rest/getStarred?u=%s&t=%s&s=%s&f=json&v=%s&c=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName);
            break;
        case OPENSUBSONIC_ENDPOINT_GETSONG:
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(getSong) ID is null.");
                // TODO handle error
                break;
            }
            rc = asprintf(&url, "%s://%s/rest/getSong?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id);
            break;
        case OPENSUBSONIC_ENDPOINT_STREAM: // Does not have a fetchResponse counterpart
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(stream) ID is null.");
                // TODO handle error
                break;
            }
            rc = asprintf(&url, "%s://%s/rest/stream?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id);
            break;
        case OPENSUBSONIC_ENDPOINT_GETCOVERART: // Does not have a fetchResponse counterpart
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(getCoverArt) ID is null.");
                // TODO handle error
                break;
            }
            rc = asprintf(&url, "%s://%s/rest/getCoverArt?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id);
            break;
        case OPENSUBSONIC_ENDPOINT_GETALBUM:
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(getAlbum) ID is null.");
                // TODO handle error
                break;
            }
            rc = asprintf(&url, "%s://%s/rest/getAlbum?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id);
            break;
        case OPENSUBSONIC_ENDPOINT_GETPLAYLISTS:
            rc = asprintf(&url, "%s://%s/rest/getPlaylists?u=%s&t=%s&s=%s&f=json&v=%s&c=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName);
            break;
        case OPENSUBSONIC_ENDPOINT_GETPLAYLIST:
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(getPlaylist) ID is null.");
                // TODO handle error
                break;
            }
            rc = asprintf(&url, "%s://%s/rest/getPlaylist?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id);
            break;
        case OPENSUBSONIC_ENDPOINT_GETARTISTS:
            rc = asprintf(&url, "%s://%s/rest/getArtists?u=%s&t=%s&s=%s&f=json&v=%s&c=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName);
            break;
        case OPENSUBSONIC_ENDPOINT_GETARTIST:
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(getArtist) ID is null.");
                // TODO handle error
                break;
            }
            rc = asprintf(&url, "%s://%s/rest/getArtist?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id);
            break;
        case OPENSUBSONIC_ENDPOINT_GETLYRICSBYSONGID:
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(getArtist) ID is null.");
                // TODO handle error
                break;
            }
            rc = asprintf(&url, "%s://%s/rest/getLyricsBySongId?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id);
            break;
        case OPENSUBSONIC_ENDPOINT_GETALBUMLIST:
            if ((*urlObj)->type == 0) {
                logger_log_error(__func__, "(getAlbumList) Type is 0.");
                // TODO handle error
                break;
            }
            if ((*urlObj)->amount == 0) {
                logger_log_error(__func__, "(getAlbumList) Amount is 0.");
                // TODO handle error
                break;
            }
            
            char* typeString = NULL;
            switch ((*urlObj)->type) {
                case OPENSUBSONIC_ENDPOINT_GETALBUMLIST_RANDOM:
                    rc = asprintf(&typeString, "random");
                    break;
                case OPENSUBSONIC_ENDPOINT_GETALBUMLIST_NEWEST:
                    rc = asprintf(&typeString, "newest");
                    break;
                case OPENSUBSONIC_ENDPOINT_GETALBUMLIST_HIGHEST:
                    rc = asprintf(&typeString, "highest");
                    break;
                case OPENSUBSONIC_ENDPOINT_GETALBUMLIST_FREQUENT:
                    rc = asprintf(&typeString, "frequent");
                    break;
                case OPENSUBSONIC_ENDPOINT_GETALBUMLIST_RECENT:
                    rc = asprintf(&typeString, "recent");
                    break;
                default:
                    logger_log_error(__func__, "(getAlbumList) Unknown type requested.");
                    // TODO handle error
            }
            
            rc = asprintf(&url, "%s://%s/rest/getAlbumList?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&type=%s&size=%d",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          typeString, (*urlObj)->amount);
            free(typeString);
            break;
        case OPENSUBSONIC_ENDPOINT_SCROBBLE:
            if ((*urlObj)->id == NULL) {
                logger_log_error(__func__, "(scrobble) ID is null.");
                // TODO handle error
                break;
            }
            
            char* submitString = NULL;
            if ((*urlObj)->submit) {
                rc = asprintf(&submitString, "true");
            } else {
                rc = asprintf(&submitString, "false");
            }
            rc = asprintf(&url, "%s://%s/rest/scrobble?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s&submission=%s",
                          configObj->opensubsonic_protocol, configObj->opensubsonic_server, configObj->opensubsonic_username,
                          configObj->internal_opensubsonic_loginToken, configObj->internal_opensubsonic_loginSalt,
                          configObj->internal_opensubsonic_version, configObj->internal_opensubsonic_clientName,
                          (*urlObj)->id, submitString);
            free(submitString);
            break;
        default:
            logger_log_error(__func__, "Unknown endpoint requested.");
            break;
    }
    
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() error.");
        // TODO handle error
    }
    
    // HACK
    (*urlObj)->formedUrl = strdup(url); free(url);
}

void opensubsonic_httpClient_fetchResponse(opensubsonic_httpClient_URL_t** urlObj, void** responseObj) {
    // Make and prepare HTTP object
    opensubsonic_httpClientRequest_t* httpReq;
    opensubsonic_httpClient_prepareRequest(&httpReq);
    httpReq->method = HTTP_METHOD_GET;
    httpReq->requestUrl = strdup((*urlObj)->formedUrl);
    opensubsonic_httpClient_request(&httpReq);
        
    // Cannot use a switch statement here due to maintaining compatibility with < C23
    if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_PING) {
        opensubsonic_ping_struct** pingStruct = (opensubsonic_ping_struct**)responseObj;
        opensubsonic_ping_parse(httpReq->responseMsg, pingStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETSTARRED) {
        opensubsonic_getStarred_struct** getStarredStruct = (opensubsonic_getStarred_struct**)responseObj;
        opensubsonic_getStarred_parse(httpReq->responseMsg, getStarredStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETSONG) {
        opensubsonic_getSong_struct** getSongStruct = (opensubsonic_getSong_struct**)responseObj;
        opensubsonic_getSong_parse(httpReq->responseMsg, getSongStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETALBUM) {
        opensubsonic_getAlbum_struct** getAlbumStruct = (opensubsonic_getAlbum_struct**)responseObj;
        opensubsonic_getAlbum_parse(httpReq->responseMsg, getAlbumStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETPLAYLISTS) {
        opensubsonic_getPlaylists_struct** getPlaylistsStruct = (opensubsonic_getPlaylists_struct**)responseObj;
        opensubsonic_getPlaylists_parse(httpReq->responseMsg, getPlaylistsStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETPLAYLIST) {
        opensubsonic_getPlaylist_struct** getPlaylistStruct = (opensubsonic_getPlaylist_struct**)responseObj;
        opensubsonic_getPlaylist_parse(httpReq->responseMsg, getPlaylistStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETARTISTS) {
        opensubsonic_getArtists_struct** getArtistsStruct = (opensubsonic_getArtists_struct**)responseObj;
        opensubsonic_getArtists_parse(httpReq->responseMsg, getArtistsStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETARTIST) {
        opensubsonic_getArtist_struct** getArtistStruct = (opensubsonic_getArtist_struct**)responseObj;
        opensubsonic_getArtist_parse(httpReq->responseMsg, getArtistStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETLYRICSBYSONGID) {
        opensubsonic_getLyricsBySongId_struct** getLyricsBySongIdStruct = (opensubsonic_getLyricsBySongId_struct**)responseObj;
        opensubsonic_getLyricsBySongId_parse(httpReq->responseMsg, getLyricsBySongIdStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_GETALBUMLIST) {
        opensubsonic_getAlbumList_struct** getAlbumListStruct = (opensubsonic_getAlbumList_struct**)responseObj;
        opensubsonic_getAlbumList_parse(httpReq->responseMsg, getAlbumListStruct);
    } else if ((*urlObj)->endpoint == OPENSUBSONIC_ENDPOINT_SCROBBLE) {
        opensubsonic_scrobble_struct** scrobbleStruct = (opensubsonic_scrobble_struct**)responseObj;
        opensubsonic_scrobble_parse(httpReq->responseMsg, scrobbleStruct);
    } else {
        logger_log_error(__func__, "Unknown endpoint requested.");
    }
    
    // Cleanup HTTP object
    opensubsonic_httpClient_cleanup(&httpReq);
}




// Contact the /rest/getAlbum endpoint
int opensubsonic_getAlbum(const char* protocol_ptr, const char* server_ptr, const char* user_ptr, char* login_token_ptr, char* login_salt_ptr, const char* opensubsonic_version_ptr, const char* client_name_ptr, char* id, char** response) {
    // Generate full URL, perform HTTP GET, and free the full URL
    int rc = 0;
    char* full_url = malloc(256);
    snprintf(full_url, 256, "%s://%s/rest/getAlbum?u=%s&t=%s&s=%s&f=json&v=%s&c=%s&id=%s", protocol_ptr, server_ptr, user_ptr, login_token_ptr, login_salt_ptr, opensubsonic_version_ptr, client_name_ptr, id);
    //rc = opensubsonic_http_json_get(full_url, response);
    free(full_url);
    return rc;
}


// TODO COVER ART - Returns JSON on error.
// {"subsonic-response":{"status":"failed","version":"1.16.1","type":"navidrome","serverVersion":"0.53.1-FREEBSD (1ba390a)","openSubsonic":true,"error":{"code":70,"message":"Artwork not found"}}}
// Contact the /rest/getCoverArt endpoint (Returns binary data)






// Functions for preparing / freeing a HTTP Request struct
void opensubsonic_httpClient_prepareRequest(opensubsonic_httpClientRequest_t** httpReq) {
    // Allocate struct
    *httpReq = (opensubsonic_httpClientRequest_t*)malloc(sizeof(opensubsonic_httpClientRequest_t));
    
    // Initialize struct variables
    (*httpReq)->requestUrl = NULL;
    (*httpReq)->requestBody = NULL;
    (*httpReq)->method = 0;
    (*httpReq)->isBodyRequired = false;
    (*httpReq)->scrobbler = 0;
    (*httpReq)->responseCode = 0;
    (*httpReq)->responseMsg = NULL;
}

void opensubsonic_httpClient_cleanup(opensubsonic_httpClientRequest_t** httpReq) {
    // Free heap-allocated struct variables
    if ((*httpReq)->requestUrl != NULL) { free((*httpReq)->requestUrl); }
    if ((*httpReq)->requestBody != NULL) { free((*httpReq)->requestBody); }
    if ((*httpReq)->responseMsg != NULL) { free((*httpReq)->responseMsg); }
    
    // Free struct
    free(*httpReq);
}

// Perform HTTP POST for Scrobbling (This function is a wrapper around OS-specific networking functions)
int opensubsonic_httpClient_request(opensubsonic_httpClientRequest_t** httpReq) {
    logger_log_general(__func__, "Performing HTTP Request.");
//#if defined(__APPLE__) && defined(__MACH__)
    //XNU_HttpRequest(httpReq);
//#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    UNIX_HttpRequest(httpReq);
//#endif
    return 0;
}







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

void UNIX_HttpRequest(opensubsonic_httpClientRequest_t** httpReq) {
    CURL* curl_handle = curl_easy_init();
    struct curl_slist* header_list = NULL;
    struct memory chunk = {0};
    long httpCode = 0;

    if (curl_handle) {
        // Set method
        if ((*httpReq)->method == HTTP_METHOD_GET) {
            curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "GET");
        } else if ((*httpReq)->method == HTTP_METHOD_POST) {
            curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "POST");
            if ((*httpReq)->isBodyRequired == false) {
                header_list = curl_slist_append(header_list, "Content-Length: 0");
            }
        }
        
        // Set scrobbler information
        if ((*httpReq)->scrobbler == SCROBBLER_LISTENBRAINZ && (*httpReq)->method == HTTP_METHOD_POST) {
            header_list = curl_slist_append(header_list, "Content-Type: application/json");
            char* authString = NULL;
            rc = asprintf(&authString, "Authorization: Token %s", configObj->listenbrainz_token);
            if (rc == -1) {
                logger_log_error(__func__, "asprintf() error.");
                // TODO handle error
            }
            printf("CODE: %s\n", authString);
            header_list = curl_slist_append(header_list, authString); // TODO Check does this copy the string?
            // TODO free auth string
        }
        
        if ((*httpReq)->isBodyRequired == true && (*httpReq)->scrobbler == 0) {
            header_list = curl_slist_append(header_list, "X-Organization: Hojuix");
            header_list = curl_slist_append(header_list, "X-Application: OSSP");
        }
        
        if (header_list != NULL) {
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, header_list);
        }
        
        if ((*httpReq)->isBodyRequired == true) {
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (*httpReq)->requestBody);
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (long)strlen((*httpReq)->requestBody));
        }
        
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "OSSP/1.0 (avery@hojuix.org)");
        curl_easy_setopt(curl_handle, CURLOPT_URL, (*httpReq)->requestUrl);
        curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE,0L);

// Do not use SSL verification on iOS due to an SSL issue with using libcurl on iOS
#if defined(__APPLE__) && defined(__MACH__) && defined(XCODE)
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
#else
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 1L);
#endif // defined(__APPLE__) && defined(__MACH__) && defined(XCODE)

        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_to_memory);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        CURLcode res = curl_easy_perform(curl_handle);
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpCode);
    }

    curl_easy_cleanup(curl_handle);

    (*httpReq)->responseMsg = strdup(chunk.data);
    (*httpReq)->responseCode = (int)httpCode;
    free(chunk.data);
}

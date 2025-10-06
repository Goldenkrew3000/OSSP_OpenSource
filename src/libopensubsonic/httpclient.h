#ifndef _HTTPCLIENT_H
#define _HTTPCLIENT_H
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define SCROBBLER_LISTENBRAINZ 101
#define SCROBBLER_LASTFM 102
#define HTTP_METHOD_GET 201
#define HTTP_METHOD_POST 202
#define HTTP_CODE_SUCCESS 200
#define HTTP_CODE_NOT_AUTHORIZED 403
#define OPENSUBSONIC_ENDPOINT_PING 301
#define OPENSUBSONIC_ENDPOINT_GETSTARRED 302
#define OPENSUBSONIC_ENDPOINT_GETSONG 303
#define OPENSUBSONIC_ENDPOINT_STREAM 304
#define OPENSUBSONIC_ENDPOINT_GETCOVERART 305
#define OPENSUBSONIC_ENDPOINT_GETALBUM 306
#define OPENSUBSONIC_ENDPOINT_GETPLAYLISTS 307
#define OPENSUBSONIC_ENDPOINT_GETPLAYLIST 308
#define OPENSUBSONIC_ENDPOINT_GETARTISTS 309
#define OPENSUBSONIC_ENDPOINT_GETARTIST 310
#define OPENSUBSONIC_ENDPOINT_GETLYRICSBYSONGID 311
#define OPENSUBSONIC_ENDPOINT_GETALBUMLIST 312
#define OPENSUBSONIC_ENDPOINT_SCROBBLE 313
#define OPENSUBSONIC_ENDPOINT_GETALBUMLIST_RANDOM 501
#define OPENSUBSONIC_ENDPOINT_GETALBUMLIST_NEWEST 502
#define OPENSUBSONIC_ENDPOINT_GETALBUMLIST_HIGHEST 503
#define OPENSUBSONIC_ENDPOINT_GETALBUMLIST_FREQUENT 504
#define OPENSUBSONIC_ENDPOINT_GETALBUMLIST_RECENT 505

typedef struct {
    int endpoint;       // Endpoint
    char* id;           // ID
    int type;           // Type of request (As used in the /getAlbumList endpoint)
    int amount;         // Amount of items to return (Also as used in the /getAlbumList endpoint)
    bool submit;        // Submit scrobble (used for the /scrobble endpoint)
    char* formedUrl;    // Final URL
} opensubsonic_httpClient_URL_t; // Forms authenticated URLs with required parameters

typedef struct {
    // Request Information
    char* requestUrl;
    char* requestBody;
    int method;
    bool isBodyRequired;
    int scrobbler;
    
    // Response Information
    int responseCode;
    char* responseMsg;
} opensubsonic_httpClientRequest_t; // OS-agnostic HTTP interface

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
void UNIX_HttpRequest(opensubsonic_httpClientRequest_t** httpReq);
#endif

void opensubsonic_httpClient_URL_prepare(opensubsonic_httpClient_URL_t** urlObj);
void opensubsonic_httpClient_URL_cleanup(opensubsonic_httpClient_URL_t** urlObj);
void opensubsonic_httpClient_formUrl(opensubsonic_httpClient_URL_t** urlObj);
void opensubsonic_httpClient_fetchResponse(opensubsonic_httpClient_URL_t** urlObj, void** responseObj);
void opensubsonic_httpClient_prepareRequest(opensubsonic_httpClientRequest_t** httpReq);
void opensubsonic_httpClient_cleanup(opensubsonic_httpClientRequest_t** httpReq);
int opensubsonic_httpClient_request(opensubsonic_httpClientRequest_t** httpReq);

void UNIX_HttpRequest(opensubsonic_httpClientRequest_t** httpReq);

// DEPRECATED - TO BE REMOVED SOON - APART OF THE OLD INFRASTRUCTURE
typedef struct {
    char* memory;
    size_t size;
} binary_response_struct;

int opensubsonic_getAlbum(const char* protocol_ptr, const char* server_ptr, const char* user_ptr, char* login_token_ptr, char* login_salt_ptr, const char* opensubsonic_version_ptr, const char* client_name_ptr, char* id, char** response);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _HTTPCLIENT_H

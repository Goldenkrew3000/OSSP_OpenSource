#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "scrobble_lastFm.h"
#include "logger.h"
#include "httpclient.h"
#include "../external/cJSON.h"
#include "../external/md5.h"
#include "../external/libcurl_uriescape.h"
#include "../configHandler.h"
#include "../configHandler.h"

const char* lastFmScrobbleURL = "https://ws.audioscrobbler.com/2.0/";
static int rc = 0;
extern configHandler_config_t* configObj;
 
/*
 * Authenticate with LastFM using the method.getMobileSession endpoint
 * Receives credentials from the config file, and returns the token as a heap allocated char*, or NULL if failure
 */
char* opensubsonic_authenticate_lastFm(void) {
    logger_log_general(__func__, "Attempting to authenticate with LastFM.");
    
    // Check to make sure there are credentials present
    if (configObj->lastfm_username == NULL || configObj->lastfm_username[0] == '\0' ||
        configObj->lastfm_password == NULL || configObj->lastfm_password[0] == '\0' ||
        configObj->lastfm_api_key == NULL || configObj->lastfm_api_key[0] == '\0' ||
        configObj->lastfm_api_secret == NULL || configObj->lastfm_api_secret[0] == '\0') {
        logger_log_error(__func__, "LastFM Username/Password/API Key/API Secret is not in the config file.");
        return NULL;
    }
    
    // Assemble the signature
    char* sig_plaintext = NULL;
    rc = asprintf(&sig_plaintext, "api_key%smethodauth.getMobileSessionpassword%susername%s%s",
                  configObj->lastfm_api_key, configObj->lastfm_password,
                  configObj->lastfm_username, configObj->lastfm_api_secret);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return NULL;
    }
    
    uint8_t sig_md5_bytes[16];
    char* sig_md5_text = NULL;
    md5String(sig_plaintext, sig_md5_bytes);
    free(sig_plaintext);
    rc = asprintf(&sig_md5_text, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                  sig_md5_bytes[0], sig_md5_bytes[1], sig_md5_bytes[2], sig_md5_bytes[3],
                  sig_md5_bytes[4], sig_md5_bytes[5], sig_md5_bytes[6], sig_md5_bytes[7],
                  sig_md5_bytes[8], sig_md5_bytes[9], sig_md5_bytes[10], sig_md5_bytes[11],
                  sig_md5_bytes[12], sig_md5_bytes[13], sig_md5_bytes[14], sig_md5_bytes[15]);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return NULL;
    }
    
    // Assemble the payload
    char* payload = NULL;
    rc = asprintf(&payload, "%s?method=auth.getMobileSession&api_key=%s&username=%s&password=%s&api_sig=%s&format=json",
                  lastFmScrobbleURL, configObj->lastfm_api_key, configObj->lastfm_username,
                  configObj->lastfm_password, sig_md5_text);
    free(sig_md5_text);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return NULL;
    }
    
    // Send payload and receive JSON response
    opensubsonic_httpClientRequest_t* httpReq;
    opensubsonic_httpClient_prepareRequest(&httpReq);
    
    httpReq->requestUrl = strdup(payload);
    free(payload);
    httpReq->scrobbler = SCROBBLER_LASTFM;
    httpReq->method = HTTP_METHOD_POST;
    opensubsonic_httpClient_request(&httpReq);
    
    if (httpReq->responseCode != HTTP_CODE_SUCCESS && httpReq->responseCode != HTTP_CODE_NOT_AUTHORIZED) {
        logger_log_error(__func__, "HTTP POST returned invalid code (%d).", httpReq->responseCode);
        opensubsonic_httpClient_cleanup(&httpReq);
        // TODO return error
    }
    
    // Parse response JSON
    cJSON* root = cJSON_Parse(httpReq->responseMsg);
    if (root == NULL) {
        logger_log_error(__func__, "Error parsing JSON.");
        opensubsonic_httpClient_cleanup(&httpReq);
        // TODO return error
    }
    
    if (httpReq->responseCode == HTTP_CODE_SUCCESS) {
        // Make an object from session
        cJSON* session_root = cJSON_GetObjectItemCaseSensitive(root, "session");
        if (session_root == NULL) {
            logger_log_error(__func__, "Error parsing JSON - session does not exist.");
            opensubsonic_httpClient_cleanup(&httpReq);
            cJSON_Delete(root);
            // TODO return error
        }
        
        cJSON* name = cJSON_GetObjectItemCaseSensitive(session_root, "name");
        if (cJSON_IsString(name) && name->valuestring != NULL) {
            printf("Fetched username: %s\n", name->valuestring);
        }
        
        cJSON* key = cJSON_GetObjectItemCaseSensitive(session_root, "key");
        if (cJSON_IsString(key) && key->valuestring != NULL) {
            printf("Fetched key: %s\n", key->valuestring);
        } else {
            logger_log_error(__func__, "Error parsing JSON - key does not exist.");
            opensubsonic_httpClient_cleanup(&httpReq);
            cJSON_Delete(root);
            // TODO return error
        }
    } else if (httpReq->responseCode == HTTP_CODE_NOT_AUTHORIZED) {
        cJSON* error = cJSON_GetObjectItemCaseSensitive(root, "error");
        if (cJSON_IsNumber(error)) {
            int code = error->valueint;
            switch (code) {
                case 2:
                    logger_log_error(__func__, "Invalid service (%d).", code);
                    break;
                case 3:
                    logger_log_error(__func__, "Invalid method (%d).", code);
                    break;
                case 4:
                    logger_log_error(__func__, "Auth error (%d).", code);
                    break;
                case 5:
                    logger_log_error(__func__, "Invalid format (%d).", code);
                    break;
                case 6:
                    logger_log_error(__func__, "Invalid parameters (%d).", code);
                    break;
                case 7:
                    logger_log_error(__func__, "Invalid resource (%d).", code);
                    break;
                case 8:
                    logger_log_error(__func__, "Operation failed (%d).", code);
                    break;
                case 9:
                    logger_log_error(__func__, "Invalid session key (%d).", code);
                    break;
                case 10:
                    logger_log_error(__func__, "Invalid API key (%d).", code);
                    break;
                case 11:
                    logger_log_error(__func__, "Service offline (%d).", code);
                    break;
                case 13:
                    logger_log_error(__func__, "Invalid method signature (%d).", code);
                    break;
                case 16:
                    logger_log_error(__func__, "Temporary error processing request (%d).", code);
                    break;
                case 26:
                    logger_log_error(__func__, "Suspended API key (%d).", code);
                    break;
                case 29:
                    logger_log_error(__func__, "Rate limit exceeded (%d).", code);
                    break;
                default:
                    logger_log_error(__func__, "Unknown error (%d).", code);
                    break;
            }
        }
        
    }
    
    opensubsonic_httpClient_cleanup(&httpReq);
    cJSON_Delete(root);
    
    // TODO fix error codes, do something with the code etc
    return NULL;
}

/*
 * Sends a scrobble to LastFM
 * If 'finalize' is true, it sends to the track.scrobble endpoint
 * If 'finalize' is false, it sends to the track.updateNowPlaying endpoint
 * Returns 1 if error, else 0
 */
int opensubsonic_scrobble_lastFm(bool finalize, opensubsonic_getSong_struct* songStruct) {
    if (finalize) {
        logger_log_general(__func__, "Performing final scrobble to LastFM.");
    } else {
        logger_log_general(__func__, "Performing in-progress scrobble to LastFM.");
    }
    
    // Fetch the current UNIX timestamp
    time_t currentTime;
    char* currentTime_string;
    currentTime = time(NULL);
    rc = asprintf(&currentTime_string, "%ld", currentTime);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed (Could not make char* of UNIX timestamp).");
        return 1;
    }
    
    // Assemble the signature
    char* sig_plaintext = NULL;
    if (finalize) {
        rc = asprintf(&sig_plaintext, "album%salbumArtist%sapi_key%sartist%smethodtrack.scrobblesk%stimestamp%strack%s%s",
                      songStruct->album, songStruct->artist, configObj->lastfm_api_key, songStruct->artist,
                      configObj->lastfm_api_session_key, currentTime_string, songStruct->title, configObj->lastfm_api_secret);
    } else {
        rc = asprintf(&sig_plaintext, "album%salbumArtist%sapi_key%sartist%smethodtrack.updateNowPlayingsk%stimestamp%strack%s%s",
                      songStruct->album, songStruct->artist, configObj->lastfm_api_key, songStruct->artist,
                      configObj->lastfm_api_session_key, currentTime_string, songStruct->title, configObj->lastfm_api_secret);
    }
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed (Could not assemble plaintext signature).");
        return 1;
    }
    
    uint8_t sig_md5_bytes[16];
    char* sig_md5_text = NULL; // TODO do I have to free this? Also is be used in crypto.c
    md5String(sig_plaintext, sig_md5_bytes);
    free(sig_plaintext);
    rc = asprintf(&sig_md5_text, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                  sig_md5_bytes[0], sig_md5_bytes[1], sig_md5_bytes[2], sig_md5_bytes[3],
                  sig_md5_bytes[4], sig_md5_bytes[5], sig_md5_bytes[6], sig_md5_bytes[7],
                  sig_md5_bytes[8], sig_md5_bytes[9], sig_md5_bytes[10], sig_md5_bytes[11],
                  sig_md5_bytes[12], sig_md5_bytes[13], sig_md5_bytes[14], sig_md5_bytes[15]);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed (Could not assemble md5 signature).");
        return 1;
    }
    
    // URI encode strings
    char* uri_songTitle = lcue_uriescape(songStruct->title, (unsigned int)strlen(songStruct->title));
    char* uri_songArtist = lcue_uriescape(songStruct->artist, (unsigned int)strlen(songStruct->artist));
    char* uri_songAlbum = lcue_uriescape(songStruct->album, (unsigned int)strlen(songStruct->album));
    if (uri_songTitle == NULL || uri_songArtist == NULL || uri_songAlbum == NULL) {
        logger_log_error(__func__, "lcue_uriescape() error (Could not URI escape required strings).");
        free(currentTime_string);
        free(sig_md5_text);
        if (uri_songTitle != NULL) { free(uri_songTitle); }
        if (uri_songArtist != NULL) { free(uri_songArtist); }
        if (uri_songAlbum != NULL) { free(uri_songAlbum); }
        return 1;
    }
    
    // Assemble the payload
    char* payload = NULL;
    if (finalize) {
        rc = asprintf(&payload,
                      "%s?method=track.scrobble&api_key=%s&timestamp=%s&track=%s&artist=%s&album=%s&albumArtist=%s&sk=%s&api_sig=%s&format=json",
                      lastFmScrobbleURL, configObj->lastfm_api_key, currentTime_string, uri_songTitle,
                      uri_songArtist, uri_songAlbum, uri_songArtist,
                      configObj->lastfm_api_session_key, sig_md5_text);
    } else {
        rc = asprintf(&payload,
                      "%s?method=track.updateNowPlaying&api_key=%s&timestamp=%s&track=%s&artist=%s&album=%s&albumArtist=%s&sk=%s&api_sig=%s&format=json",
                      lastFmScrobbleURL, configObj->lastfm_api_key, currentTime_string, uri_songTitle,
                      uri_songArtist, uri_songAlbum, uri_songArtist,
                      configObj->lastfm_api_session_key, sig_md5_text);
    }
    free(currentTime_string);
    free(sig_md5_text);
    free(uri_songTitle);
    free(uri_songAlbum);
    free(uri_songArtist);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed (Could not assemble payload).");
        return 1;
    }
    
    // Send scrobble and receive response
    opensubsonic_httpClientRequest_t* httpReq;
    opensubsonic_httpClient_prepareRequest(&httpReq);
    
    httpReq->requestUrl = strdup(payload);
    free(payload);
    httpReq->scrobbler = SCROBBLER_LASTFM;
    httpReq->method = HTTP_METHOD_POST;
    opensubsonic_httpClient_request(&httpReq);
    
    if (httpReq->responseCode != HTTP_CODE_SUCCESS) {
        logger_log_error(__func__, "HTTP POST did not return success (%d).", httpReq->responseCode);
        opensubsonic_httpClient_cleanup(&httpReq);
        // TODO return error
    }
    
    // Parse the scrobble response
    cJSON* root = cJSON_Parse(httpReq->responseMsg);
    opensubsonic_httpClient_cleanup(&httpReq);
    if (root == NULL) {
        logger_log_error(__func__, "Error parsing JSON.");
        // TODO return error
    }
    
    cJSON* inner_root = NULL;
    cJSON* scrobbles_root = NULL; // Parent of inner_root, only used on final scrobble
    if (finalize) {
        // Make an object from scrobbles
        scrobbles_root = cJSON_GetObjectItemCaseSensitive(root, "scrobbles");
        if (scrobbles_root == NULL) {
            logger_log_error(__func__, "Error parsing JSON - scrobbles does not exist.");
            cJSON_Delete(root);
            // TODO return error
        }
        
        // Make an object from scrobble
        inner_root = cJSON_GetObjectItemCaseSensitive(scrobbles_root, "scrobble");
        if (inner_root == NULL) {
            logger_log_error(__func__, "Error parsing JSON - scrobble does not exist.");
            cJSON_Delete(root);
            // TODO return error
        }
    } else {
        // Make an object from nowplaying
        inner_root = cJSON_GetObjectItemCaseSensitive(root, "nowplaying");
        if (inner_root == NULL) {
            logger_log_error(__func__, "Error parsing JSON - nowplaying does not exist.");
            cJSON_Delete(root);
            // TODO return error
        }
    }
    
    // Make an object from artist, track, albumArtist, and album, and fetch codes
    cJSON* artist_root = cJSON_GetObjectItemCaseSensitive(inner_root, "artist");
    if (artist_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - artist does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* artist_corrected = cJSON_GetObjectItemCaseSensitive(artist_root, "corrected");
    if (cJSON_IsString(artist_corrected) && artist_corrected->valuestring != NULL) {
        if (*(artist_corrected->valuestring) == '\0') {
            logger_log_error(__func__, "Error parsing JSON - artist/corrected is empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        char* endptr;
        int corrected = (int)strtol(artist_corrected->valuestring, &endptr, 10);
        if (*endptr != '\0') {
            logger_log_error(__func__, "Error parsing JSON - artist/corrected strtol/endptr is not empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        if (corrected == 1) {
            logger_log_important(__func__, "Warning - Artist has been autocorrected.");
        }
    } else {
        logger_log_error(__func__, "Error parsing JSON - artist/corrected does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* track_root = cJSON_GetObjectItemCaseSensitive(inner_root, "track");
    if (track_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - track does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* track_corrected = cJSON_GetObjectItemCaseSensitive(track_root, "corrected");
    if (cJSON_IsString(track_corrected) && track_corrected->valuestring != NULL) {
        if (*(track_corrected->valuestring) == '\0') {
            logger_log_error(__func__, "Error parsing JSON - track/corrected is empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        char* endptr;
        int corrected = (int)strtol(track_corrected->valuestring, &endptr, 10);
        if (*endptr != '\0') {
            logger_log_error(__func__, "Error parsing JSON - track/corrected strtol/endptr is not empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        if (corrected == 1) {
            logger_log_important(__func__, "Warning - Track has been autocorrected.");
        }
    } else {
        logger_log_error(__func__, "Error parsing JSON - track/corrected does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* albumArtist_root = cJSON_GetObjectItemCaseSensitive(inner_root, "albumArtist");
    if (albumArtist_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - albumArtist does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* albumArtist_corrected = cJSON_GetObjectItemCaseSensitive(albumArtist_root, "corrected");
    if (cJSON_IsString(albumArtist_corrected) && albumArtist_corrected->valuestring != NULL) {
        if (*(albumArtist_corrected->valuestring) == '\0') {
            logger_log_error(__func__, "Error parsing JSON - albumArtist/corrected is empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        char* endptr;
        int corrected = (int)strtol(albumArtist_corrected->valuestring, &endptr, 10);
        if (*endptr != '\0') {
            logger_log_error(__func__, "Error parsing JSON - albumArtist/corrected strtol/endptr is not empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        if (corrected == 1) {
            logger_log_important(__func__, "Warning - Album Artist has been autocorrected.");
        }
    } else {
        logger_log_error(__func__, "Error parsing JSON - albumArtist/corrected does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* album_root = cJSON_GetObjectItemCaseSensitive(inner_root, "album");
    if (album_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - album does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* album_corrected = cJSON_GetObjectItemCaseSensitive(album_root, "corrected");
    if (cJSON_IsString(album_corrected) && album_corrected->valuestring != NULL) {
        if (*(album_corrected->valuestring) == '\0') {
            logger_log_error(__func__, "Error parsing JSON - album/corrected is empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        char* endptr;
        int corrected = (int)strtol(album_corrected->valuestring, &endptr, 10);
        if (*endptr != '\0') {
            logger_log_error(__func__, "Error parsing JSON - album/corrected strtol/endptr is not empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        if (corrected == 1) {
            logger_log_important(__func__, "Warning - Album has been autocorrected.");
        }
    } else {
        logger_log_error(__func__, "Error parsing JSON - album/corrected does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    // Make an object from ignoredMessage, and check return code
    cJSON* ignoredMessage_root = cJSON_GetObjectItemCaseSensitive(inner_root, "ignoredMessage");
    if (ignoredMessage_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - ignoredMessage does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }
    
    cJSON* ignoredMessage_code = cJSON_GetObjectItemCaseSensitive(ignoredMessage_root, "code");
    if (cJSON_IsString(ignoredMessage_code) && ignoredMessage_code->valuestring != NULL) {
        if (*(ignoredMessage_code->valuestring) == '\0') {
            logger_log_error(__func__, "Error parsing JSON - ignoredMessage/code is empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        char* endptr;
        int code = (int)strtol(ignoredMessage_code->valuestring, &endptr, 10);
        if (*endptr != '\0') {
            logger_log_error(__func__, "Error parsing JSON - ignoredMessage/code strtol/endptr is not empty.");
            cJSON_Delete(root);
            // TODO return error
        }
        if (code == 0) {
            if (!finalize) {
                logger_log_general(__func__, "In progress scrobble was successful.");
            } else {
                logger_log_general(__func__, "Final scrobble 1/2 was successful.");
            }
        } else if (code == 1) {
            logger_log_error(__func__, "Artist was ignored.");
        } else if (code == 2) {
            logger_log_error(__func__, "Track was ignored.");
        } else if (code == 3) {
            logger_log_error(__func__, "Timestamp was too old.");
        } else if (code == 4) {
            logger_log_error(__func__, "Timestamp was too new.");
        } else if (code == 5) {
            logger_log_error(__func__, "Daily scrobble limit exceeded.");
        } else {
            logger_log_error(__func__, "Unknown error code received (%d)", code);
        }
    } else {
        logger_log_error(__func__, "Error parsing JSON - ignoredMessage/code does not exist.");
        cJSON_Delete(root);
        // TODO return error
    }

    if (finalize) {
        cJSON* attr_root = cJSON_GetObjectItemCaseSensitive(scrobbles_root, "@attr");
        if (attr_root == NULL) {
            logger_log_error(__func__, "Error parsing JSON - @attr does not exist.");
            cJSON_Delete(root);
            // TODO return error
        }
        
        cJSON* attr_ignored = cJSON_GetObjectItemCaseSensitive(attr_root, "ignored");
        if (cJSON_IsNumber(attr_ignored)) {
            if (attr_ignored->valueint != 0) {
                logger_log_important(__func__, "Warning - @attr/ignored is not 0 (%d).", attr_ignored->valueint);
            }
        } else {
            logger_log_error(__func__, "Error parsing JSON - @attr/ignored does not exist.");
            cJSON_Delete(root);
            // TODO return error
        }
        
        cJSON* attr_accepted = cJSON_GetObjectItemCaseSensitive(attr_root, "accepted");
        if (cJSON_IsNumber(attr_accepted)) {
            if (attr_accepted->valueint != 1) {
                logger_log_important(__func__, "Warning - @attr/accepted is not 1 (%d).", attr_accepted->valueint);
            }
        } else {
            logger_log_error(__func__, "Error parsing JSON - @attr/accepted does not exist");
            cJSON_Delete(root);
            // TODO return error
        }
        
        // At this point, attr_ignored and attr_accepted are both known to be valid
        if (attr_ignored->valueint == 0 && attr_accepted->valueint == 1) {
            logger_log_general(__func__, "Final scrobble 2/2 was successful.");
        } else {
            logger_log_important(__func__, "Final scobble 2/2 was not successful (ignored: %d, accepted: %d",
                                 attr_ignored->valueint, attr_accepted->valueint);
        }
    }
    
    cJSON_Delete(root);
}

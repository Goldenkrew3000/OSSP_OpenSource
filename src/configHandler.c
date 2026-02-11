/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Configuration Handler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "configHandler.h"
#include "external/cJSON.h"
#include "libopensubsonic/logger.h"

#if defined(__APPLE__) && defined(__MACH__) && defined(XCODE)
#include "OSSP_Bridge.h"
#endif // defined(__APPLE__) && defined(__MACH__) && defined(__XCODE__)

static int rc = 0;

/*
 * Read a predefined config file into the configuration struct
 * Returns 0 on success, 1 on failure
 */
int configHandler_Read(configHandler_config_t** configObj) {
    // Allocate config object on heap
    *configObj = malloc(sizeof(configHandler_config_t));

    // Initialize struct variables
    (*configObj)->opensubsonic_protocol = NULL;
    (*configObj)->opensubsonic_server = NULL;
    (*configObj)->opensubsonic_username = NULL;
    (*configObj)->opensubsonic_password = NULL;
    (*configObj)->internal_opensubsonic_version = NULL;
    (*configObj)->internal_opensubsonic_clientName = NULL;
    (*configObj)->internal_opensubsonic_loginSalt = NULL;
    (*configObj)->internal_opensubsonic_loginToken = NULL;
    (*configObj)->internal_ossp_version = NULL;
    (*configObj)->listenbrainz_enable = false;
    (*configObj)->listenbrainz_token = NULL;
    (*configObj)->lastfm_enable = false;
    (*configObj)->lastfm_username = NULL;
    (*configObj)->lastfm_password = NULL;
    (*configObj)->lastfm_api_key = NULL;
    (*configObj)->lastfm_api_secret = NULL;
    (*configObj)->lastfm_api_session_key = NULL;
    (*configObj)->discordrpc_enable = false;
    (*configObj)->discordrpc_method = 0;
    (*configObj)->discordrpc_showSysDetails = false;
    (*configObj)->discordrpc_showCoverArt = false;
    (*configObj)->audio_equalizer_enable = false;
    (*configObj)->audio_equalizer_followPitch = false;
    (*configObj)->audio_equalizer_graphCount = 0;
    (*configObj)->audio_equalizer_graph = NULL;
    (*configObj)->audio_pitch_enable = false;
    (*configObj)->audio_pitch_cents = 0.00;
    (*configObj)->audio_pitch_rate = 0.00;
    (*configObj)->audio_reverb_enable = false;
    (*configObj)->audio_reverb_wetDryMix = 0.00;
    (*configObj)->lv2_parax32_filter_name = NULL;
    (*configObj)->lv2_parax32_filter_type_left = NULL;
    (*configObj)->lv2_parax32_filter_type_right = NULL;
    (*configObj)->lv2_parax32_gain_left = NULL;
    (*configObj)->lv2_parax32_gain_right = NULL;
    (*configObj)->lv2_parax32_quality_left = NULL;
    (*configObj)->lv2_parax32_quality_right = NULL;
    (*configObj)->lv2_parax32_frequency_left = NULL;
    (*configObj)->lv2_parax32_frequency_right = NULL;
    (*configObj)->lv2_reverb_filter_name = NULL;

    // Set internal configuration values
    (*configObj)->internal_opensubsonic_version = strdup("1.8.0");
    (*configObj)->internal_opensubsonic_clientName = strdup("Hojuix_OSSP");
    (*configObj)->internal_ossp_version = strdup("v0.4a");

    // Form the path to the config JSON
    char* config_path = NULL;
#if defined(__APPLE__) && defined(__MACH__) && defined(XCODE)
    // NOTE: This is a relatively hacky way of fetching the iOS container path without diving into the hell that is ObjC
    char* root_path = getenv("HOME");
    rc = asprintf(&config_path, "%s/Documents/config.json", root_path);
#if DEBUG
    printf("iOS Container Path: %s\n", config_path);
#endif // DEBUG
#else
    char* root_path = getenv("HOME");
    rc = asprintf(&config_path, "%s/.config/ossp/config.json", root_path);
#endif // defined(__APPLE__) && defined(__MACH__) && defined(XCODE)
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed (Could not generate config path).");
        free(config_path);
        return 1;
    }

    // Read config file
    FILE* config_fd = NULL;
    char* config_buf = NULL;
    long config_fsize = 0;
    long config_fread_rc = 0; // Needs to be separate from 'rc' as fread() returns bytes read

    // Check if the config file exists, and fetch it's size
    struct stat st;
    if (stat(config_path, &st) == 0) {
        config_fsize = st.st_size;
    } else {
        logger_log_error(__func__, "stat() failed (Config file does not exist).");
        logger_log_error(__func__, "This is most likely because your config file does not exist.");
        logger_log_error(__func__, "Config file should be located at ~/.config/ossp/config.json");
        return 1;
    }

    // Actually open and read in the contents of the config file
    config_fd = fopen(config_path, "rb");
    if (config_fd == NULL) {
        logger_log_error(__func__, "fopen() failed (Could not open config file).");
        free(config_path);
        return 1;
    }
    free(config_path);

    config_buf = (char*)malloc(config_fsize + 1);
    if (config_buf == NULL) {
        logger_log_error(__func__, "malloc() failed (Could not allocate enough memory for the config file).");
        fclose(config_fd);
        return 1;
    }

    config_fread_rc = fread(config_buf, 1, config_fsize, config_fd);
    if (config_fread_rc != config_fsize) {
        logger_log_error(__func__, "fread() failed (Could not read the config file).");
        fclose(config_fd);
        free(config_buf);
        return 1;
    }

    // Null terminate the buffer
    config_buf[config_fsize] = '\0';
    fclose(config_fd);

    // Parse config JSON
    cJSON* root = cJSON_Parse(config_buf);
    free(config_buf);
    if (root == NULL) {
        logger_log_error(__func__, "cJSON_Parse() failed (Could not parse the configuration JSON).");
        return 1;
    }

    // Make an object from opensubsonic_server
    // TODO - Use the new OSS_P*oj functions?
    cJSON* opensubsonic_server_root = cJSON_GetObjectItemCaseSensitive(root, "opensubsonic_server");
    if (opensubsonic_server_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - opensubsonic_server does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* opensubsonic_server_protocol = cJSON_GetObjectItemCaseSensitive(opensubsonic_server_root, "protocol");
    if (cJSON_IsString(opensubsonic_server_protocol) && opensubsonic_server_protocol->valuestring != NULL) {
        (*configObj)->opensubsonic_protocol = strdup(opensubsonic_server_protocol->valuestring);
    }

    cJSON* opensubsonic_server_server = cJSON_GetObjectItemCaseSensitive(opensubsonic_server_root, "server");
    if (cJSON_IsString(opensubsonic_server_server) && opensubsonic_server_server->valuestring != NULL) {
        (*configObj)->opensubsonic_server = strdup(opensubsonic_server_server->valuestring);
    }

    cJSON* opensubsonic_server_username = cJSON_GetObjectItemCaseSensitive(opensubsonic_server_root, "username");
    if (cJSON_IsString(opensubsonic_server_username) && opensubsonic_server_username->valuestring != NULL) {
        (*configObj)->opensubsonic_username = strdup(opensubsonic_server_username->valuestring);
    }

    cJSON* opensubsonic_server_password = cJSON_GetObjectItemCaseSensitive(opensubsonic_server_root, "password");
    if (cJSON_IsString(opensubsonic_server_password) && opensubsonic_server_password->valuestring != NULL) {
        (*configObj)->opensubsonic_password = strdup(opensubsonic_server_password->valuestring);
    }

    // Make an object from scrobbler
    cJSON* scrobbler_root = cJSON_GetObjectItemCaseSensitive(root, "scrobbler");
    if (scrobbler_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - scrobbler does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* scrobbler_listenbrainz_enable = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "listenbrainz_enable");
    if (cJSON_IsBool(scrobbler_listenbrainz_enable)) {
        if (cJSON_IsTrue(scrobbler_listenbrainz_enable)) {
            (*configObj)->listenbrainz_enable = true;
        }
    }

    cJSON* scrobbler_listenbrainz_token = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "listenbrainz_token");
    if (cJSON_IsString(scrobbler_listenbrainz_token) && scrobbler_listenbrainz_token->valuestring != NULL) {
        (*configObj)->listenbrainz_token = strdup(scrobbler_listenbrainz_token->valuestring);
    }

    cJSON* scrobbler_lastfm_enable = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "lastfm_enable");
    if (cJSON_IsBool(scrobbler_lastfm_enable)) {
        if (cJSON_IsTrue(scrobbler_lastfm_enable)) {
            (*configObj)->lastfm_enable = true;
        }
    }

    cJSON* scrobbler_lastfm_username = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "lastfm_username");
    if (cJSON_IsString(scrobbler_lastfm_username) && scrobbler_lastfm_username->valuestring != NULL) {
        (*configObj)->lastfm_username = strdup(scrobbler_lastfm_username->valuestring);
    }

    cJSON* scrobbler_lastfm_password = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "lastfm_password");
    if (cJSON_IsString(scrobbler_lastfm_password) && scrobbler_lastfm_password->valuestring != NULL) {
        (*configObj)->lastfm_password = strdup(scrobbler_lastfm_password->valuestring);
    }

    cJSON* scrobbler_lastfm_api_key = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "lastfm_api_key");
    if (cJSON_IsString(scrobbler_lastfm_api_key) && scrobbler_lastfm_api_key->valuestring != NULL) {
        (*configObj)->lastfm_api_key = strdup(scrobbler_lastfm_api_key->valuestring);
    }

    cJSON* scrobbler_lastfm_api_secret = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "lastfm_api_secret");
    if (cJSON_IsString(scrobbler_lastfm_api_secret) && scrobbler_lastfm_api_secret->valuestring != NULL) {
        (*configObj)->lastfm_api_secret = strdup(scrobbler_lastfm_api_secret->valuestring);
    }

    cJSON* scrobbler_lastfm_api_session_key = cJSON_GetObjectItemCaseSensitive(scrobbler_root, "lastfm_session_key");
    if (cJSON_IsString(scrobbler_lastfm_api_session_key) && scrobbler_lastfm_api_session_key->valuestring != NULL) {
        (*configObj)->lastfm_api_session_key = strdup(scrobbler_lastfm_api_session_key->valuestring);
    }

    // Make an object from discord-rpc
    cJSON* discordrpc_root = cJSON_GetObjectItemCaseSensitive(root, "discord_rpc");
    if (discordrpc_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - discord-rpc does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* discordrpc_enable = cJSON_GetObjectItemCaseSensitive(discordrpc_root, "enable");
    if (cJSON_IsBool(discordrpc_enable)) {
        if (cJSON_IsTrue(discordrpc_enable)) {
            (*configObj)->discordrpc_enable = true;
        }
    }

    cJSON* discordrpc_method = cJSON_GetObjectItemCaseSensitive(discordrpc_root, "method");
    if (cJSON_IsNumber(discordrpc_method)) {
        (*configObj)->discordrpc_method = discordrpc_method->valueint;
    }

    cJSON* discordrpc_showSysDetails = cJSON_GetObjectItemCaseSensitive(discordrpc_root, "showSystemDetails");
    if (cJSON_IsBool(discordrpc_showSysDetails)) {
        if (cJSON_IsTrue(discordrpc_showSysDetails)) {
            (*configObj)->discordrpc_showSysDetails = true;
        }
    }

    cJSON* discordrpc_showCoverArt = cJSON_GetObjectItemCaseSensitive(discordrpc_root, "showCoverArt");
    if (cJSON_IsBool(discordrpc_showCoverArt)) {
        if (cJSON_IsTrue(discordrpc_showCoverArt)) {
            (*configObj)->discordrpc_showCoverArt = true;
        }
    }

    // Make an object from audio
    cJSON* audio_root = cJSON_GetObjectItemCaseSensitive(root, "audio");
    if (audio_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - audio does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from equalizer
    cJSON* equalizer_root = cJSON_GetObjectItemCaseSensitive(audio_root, "equalizer");
    if (equalizer_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - equalizer does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* audio_equalizer_enable = cJSON_GetObjectItemCaseSensitive(equalizer_root, "enable");
    if (cJSON_IsBool(audio_equalizer_enable)) {
        if (cJSON_IsTrue(audio_equalizer_enable)) {
            (*configObj)->audio_equalizer_enable = true;
        }
    }

    cJSON* audio_equalizer_followPitch = cJSON_GetObjectItemCaseSensitive(equalizer_root, "followPitch");
    if (cJSON_IsBool(audio_equalizer_followPitch)) {
        if (cJSON_IsTrue(audio_equalizer_followPitch)) {
            (*configObj)->audio_equalizer_followPitch = true;
        }
    }

    // Fetch the equalizer graph array, and allocate memory for it
    cJSON* equalizer_graph_array = cJSON_GetObjectItemCaseSensitive(equalizer_root, "graph");
    if (equalizer_graph_array == NULL) {
        logger_log_error(__func__, "Error parsing JSON - graph does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    (*configObj)->audio_equalizer_graphCount = cJSON_GetArraySize(equalizer_graph_array);
    (*configObj)->audio_equalizer_graph = (configHandler_eqGraph_t*)malloc((*configObj)->audio_equalizer_graphCount * sizeof(configHandler_eqGraph_t));
    if ((*configObj)->audio_equalizer_graph == NULL) {
        logger_log_error(__func__, "malloc() failed.");
        cJSON_Delete(root);
        return 1;
    }

    // Initialize more struct variables
    for (size_t i = 0; i < (*configObj)->audio_equalizer_graphCount; i++) {
        (*configObj)->audio_equalizer_graph[i].bandwidth = 0.00;
        (*configObj)->audio_equalizer_graph[i].frequency = 0;
        (*configObj)->audio_equalizer_graph[i].gain = 0.00;
        (*configObj)->audio_equalizer_graph[i].bypass = false;
    }

    for (size_t i = 0; i < (*configObj)->audio_equalizer_graphCount; i++) {
        cJSON* array_equalizer_graph_root = cJSON_GetArrayItem(equalizer_graph_array, (int)i);
        if (array_equalizer_graph_root == NULL) {
            logger_log_error(__func__, "Error parsing JSON - Could not fetch graph index %d.", i);
            cJSON_Delete(root);
            return 1;
        }

        cJSON* audio_equalizer_graph_bandwidth = cJSON_GetObjectItemCaseSensitive(array_equalizer_graph_root, "bandwidth");
        if (cJSON_IsNumber(audio_equalizer_graph_bandwidth)) {
            (*configObj)->audio_equalizer_graph[i].bandwidth = audio_equalizer_graph_bandwidth->valuedouble;
        }

        cJSON* audio_equalizer_graph_frequency = cJSON_GetObjectItemCaseSensitive(array_equalizer_graph_root, "frequency");
        if (cJSON_IsNumber(audio_equalizer_graph_frequency)) {
            (*configObj)->audio_equalizer_graph[i].frequency = audio_equalizer_graph_frequency->valueint;
        }

        cJSON* audio_equalizer_graph_gain = cJSON_GetObjectItemCaseSensitive(array_equalizer_graph_root, "gain");
        if (cJSON_IsNumber(audio_equalizer_graph_gain)) {
            (*configObj)->audio_equalizer_graph[i].gain = audio_equalizer_graph_gain->valuedouble;
        }

        cJSON* audio_equalizer_graph_bypass = cJSON_GetObjectItemCaseSensitive(array_equalizer_graph_root, "bypass");
        if (cJSON_IsBool(audio_equalizer_graph_bypass)) {
            if (cJSON_IsTrue(audio_equalizer_graph_bypass)) {
                (*configObj)->audio_equalizer_graph[i].bypass = true;
            }
        }
    }

    // Make an object from pitch
    cJSON* pitch_root = cJSON_GetObjectItemCaseSensitive(audio_root, "pitch");
    if (pitch_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - pitch does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* audio_pitch_enable = cJSON_GetObjectItemCaseSensitive(pitch_root, "enable");
    if (cJSON_IsBool(audio_pitch_enable)) {
        if (cJSON_IsTrue(audio_pitch_enable)) {
            (*configObj)->audio_pitch_enable = true;
        }
    }

    cJSON* audio_pitch_cents = cJSON_GetObjectItemCaseSensitive(pitch_root, "cents");
    if (cJSON_IsNumber(audio_pitch_cents)) {
        (*configObj)->audio_pitch_cents = audio_pitch_cents->valuedouble;
    }

    cJSON* audio_pitch_rate = cJSON_GetObjectItemCaseSensitive(pitch_root, "rate");
    if (cJSON_IsNumber(audio_pitch_rate)) {
        (*configObj)->audio_pitch_rate = audio_pitch_rate->valuedouble;
    }

    // Make an object from reverb
    cJSON* reverb_root = cJSON_GetObjectItemCaseSensitive(audio_root, "reverb");
    if (reverb_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - reverb does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* audio_reverb_enable = cJSON_GetObjectItemCaseSensitive(reverb_root, "enable");
    if (cJSON_IsBool(audio_reverb_enable)) {
        if (cJSON_IsTrue(audio_reverb_enable)) {
            (*configObj)->audio_reverb_enable = true;
        }
    }

    cJSON* audio_reverb_wetDryMix = cJSON_GetObjectItemCaseSensitive(reverb_root, "wetDryMix");
    if (cJSON_IsNumber(audio_reverb_wetDryMix)) {
        (*configObj)->audio_reverb_wetDryMix = audio_reverb_wetDryMix->valuedouble;
    }

    // Make an object from lv2
    cJSON* lv2_root = cJSON_GetObjectItemCaseSensitive(audio_root, "lv2");
    if (lv2_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - lv2 does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    // Make an object from lsp_para_x32_lr
    cJSON* lsp_para_x32_lr_root = cJSON_GetObjectItemCaseSensitive(lv2_root, "lsp_para_x32_lr");
    if (lsp_para_x32_lr_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - lsp_para_x32_lr does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* lsp_para_x32_lr_filter_name = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "filter_name");
    if (cJSON_IsString(lsp_para_x32_lr_filter_name) && lsp_para_x32_lr_filter_name->valuestring != NULL) {
        (*configObj)->lv2_parax32_filter_name = strdup(lsp_para_x32_lr_filter_name->valuestring);
    }

    cJSON* lsp_para_x32_lr_filter_type_left = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "filter_type_left");
    if (cJSON_IsString(lsp_para_x32_lr_filter_type_left) && lsp_para_x32_lr_filter_type_left->valuestring != NULL) {
        (*configObj)->lv2_parax32_filter_type_left = strdup(lsp_para_x32_lr_filter_type_left->valuestring);
    }

    cJSON* lsp_para_x32_lr_filter_type_right = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "filter_type_right");
    if (cJSON_IsString(lsp_para_x32_lr_filter_type_right) && lsp_para_x32_lr_filter_type_right->valuestring != NULL) {
        (*configObj)->lv2_parax32_filter_type_right = strdup(lsp_para_x32_lr_filter_type_right->valuestring);
    }

    cJSON* lsp_para_x32_lr_gain_left = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "gain_left");
    if (cJSON_IsString(lsp_para_x32_lr_gain_left) && lsp_para_x32_lr_gain_left->valuestring != NULL) {
        (*configObj)->lv2_parax32_gain_left = strdup(lsp_para_x32_lr_gain_left->valuestring);
    }

    cJSON* lsp_para_x32_lr_gain_right = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "gain_right");
    if (cJSON_IsString(lsp_para_x32_lr_gain_right) && lsp_para_x32_lr_gain_right->valuestring != NULL) {
        (*configObj)->lv2_parax32_gain_right = strdup(lsp_para_x32_lr_gain_right->valuestring);
    }

    cJSON* lsp_para_x32_lr_quality_left = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "quality_left");
    if (cJSON_IsString(lsp_para_x32_lr_quality_left) && lsp_para_x32_lr_quality_left->valuestring != NULL) {
        (*configObj)->lv2_parax32_quality_left = strdup(lsp_para_x32_lr_quality_left->valuestring);
    }

    cJSON* lsp_para_x32_lr_quality_right = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "quality_right");
    if (cJSON_IsString(lsp_para_x32_lr_quality_right) && lsp_para_x32_lr_quality_right->valuestring != NULL) {
        (*configObj)->lv2_parax32_quality_right = strdup(lsp_para_x32_lr_quality_right->valuestring);
    }

    cJSON* lsp_para_x32_lr_frequency_left = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "frequency_left");
    if (cJSON_IsString(lsp_para_x32_lr_frequency_left) && lsp_para_x32_lr_frequency_left->valuestring != NULL) {
        (*configObj)->lv2_parax32_frequency_left = strdup(lsp_para_x32_lr_frequency_left->valuestring);
    }

    cJSON* lsp_para_x32_lr_frequency_right = cJSON_GetObjectItemCaseSensitive(lsp_para_x32_lr_root, "frequency_right");
    if (cJSON_IsString(lsp_para_x32_lr_frequency_right) && lsp_para_x32_lr_frequency_right->valuestring != NULL) {
        (*configObj)->lv2_parax32_frequency_right = strdup(lsp_para_x32_lr_frequency_right->valuestring);
    }

    // Make an object from calf_reverb
    cJSON* calf_reverb_root = cJSON_GetObjectItemCaseSensitive(lv2_root, "calf_reverb");
    if (calf_reverb_root == NULL) {
        logger_log_error(__func__, "Error parsing JSON - calf_reverb does not exist.");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* calf_reverb_filter_name = cJSON_GetObjectItemCaseSensitive(calf_reverb_root, "filter_name");
    if (cJSON_IsString(calf_reverb_filter_name) && calf_reverb_filter_name->valuestring != NULL) {
        (*configObj)->lv2_reverb_filter_name = strdup(calf_reverb_filter_name->valuestring);
    }

    cJSON_Delete(root);
    logger_log_general(__func__, "Successfully read configuration file.");
    return 0;
}

void configHandler_Free(configHandler_config_t** configObj) {
    if ((*configObj)->opensubsonic_protocol != NULL) { free((*configObj)->opensubsonic_protocol); }
    if ((*configObj)->opensubsonic_server != NULL) { free((*configObj)->opensubsonic_server); }
    if ((*configObj)->opensubsonic_username != NULL) { free((*configObj)->opensubsonic_username); }
    if ((*configObj)->opensubsonic_password != NULL) { free((*configObj)->opensubsonic_password); }
    if ((*configObj)->internal_opensubsonic_version != NULL) { free((*configObj)->internal_opensubsonic_version); }
    if ((*configObj)->internal_opensubsonic_clientName != NULL) { free((*configObj)->internal_opensubsonic_clientName); }
    if ((*configObj)->internal_opensubsonic_loginSalt != NULL) { free((*configObj)->internal_opensubsonic_loginSalt); }
    if ((*configObj)->internal_opensubsonic_loginToken != NULL) { free((*configObj)->internal_opensubsonic_loginToken); }
    if ((*configObj)->internal_ossp_version != NULL) { free((*configObj)->internal_ossp_version); }
    if ((*configObj)->listenbrainz_token != NULL) { free((*configObj)->listenbrainz_token); }
    if ((*configObj)->lastfm_username != NULL) { free((*configObj)->lastfm_username); }
    if ((*configObj)->lastfm_password != NULL) { free((*configObj)->lastfm_password); }
    if ((*configObj)->lastfm_api_key != NULL) { free((*configObj)->lastfm_api_key); }
    if ((*configObj)->lastfm_api_secret != NULL) { free((*configObj)->lastfm_api_secret); }
    if ((*configObj)->lastfm_api_session_key != NULL) { free((*configObj)->lastfm_api_session_key); }
    if ((*configObj)->audio_equalizer_graph != NULL) { free((*configObj)->audio_equalizer_graph); }
    if ((*configObj)->lv2_parax32_filter_name != NULL) { free((*configObj)->lv2_parax32_filter_name); }
    if ((*configObj)->lv2_parax32_filter_type_left != NULL) { free((*configObj)->lv2_parax32_filter_type_left); }
    if ((*configObj)->lv2_parax32_filter_type_right != NULL) { free((*configObj)->lv2_parax32_filter_type_right); }
    if ((*configObj)->lv2_parax32_gain_left != NULL) { free((*configObj)->lv2_parax32_gain_left); }
    if ((*configObj)->lv2_parax32_gain_right != NULL) { free((*configObj)->lv2_parax32_gain_right); }
    if ((*configObj)->lv2_parax32_quality_left != NULL) { free((*configObj)->lv2_parax32_quality_left); }
    if ((*configObj)->lv2_parax32_quality_right != NULL) { free((*configObj)->lv2_parax32_quality_right); }
    if ((*configObj)->lv2_parax32_frequency_left != NULL) { free((*configObj)->lv2_parax32_frequency_left); }
    if ((*configObj)->lv2_parax32_frequency_right != NULL) { free((*configObj)->lv2_parax32_frequency_right); }
    if ((*configObj)->lv2_reverb_filter_name != NULL) { free((*configObj)->lv2_reverb_filter_name); }
    if (*configObj != NULL) { free(*configObj); }
}

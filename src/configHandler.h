/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 */

#ifndef _CONFIG_HANDLER_H
#define _CONFIG_HANDLER_H
#include <stdbool.h>

typedef struct {
    double bandwidth;
    int frequency;                      // Frequency in Hz
    double gain;                        // Gain in db
    bool bypass;                        // Ignore entry
} configHandler_eqGraph_t;

typedef struct {
    // Opensubsonic Settings
    char* opensubsonic_protocol;        // http / https
    char* opensubsonic_server;          // address:port
    char* opensubsonic_username;
    char* opensubsonic_password;
    char* internal_opensubsonic_version;        // (Internal) Opensubsonic API Version
    char* internal_opensubsonic_clientName;     // (Internal) Opensubsonic Client Name
    char* internal_opensubsonic_loginSalt;      // (Internal) Opensubsonic Login Salt
    char* internal_opensubsonic_loginToken;     // (Internal) Opensubsonic Login Token

    // Scrobbler Settings
    bool listenbrainz_enable;           // Enable ListenBrainz Scrobbling
    char* listenbrainz_token;           // ListenBrainz Token
    bool lastfm_enable;                 // Enable LastFM Scrobbling
    char* lastfm_username;              // LastFM Username
    char* lastfm_password;              // LastFM Password
    char* lastfm_api_key;               // LastFM API Key
    char* lastfm_api_secret;            // LastFM API Secret
    char* lastfm_api_session_key;       // LastFM API Session Key (Generated from authorization endpoint)

    // Discord RPC Settings
    bool discordrpc_enable;             // Enable Discord RPC
    int discordrpc_method;              // Discord RPC Method (0 = Regular, 1 = DscrdRPC)
    bool discordrpc_showSysDetails;     // Show 'on OS ARCH VERSION' in RPC
    bool discordrpc_showCoverArt;       // Show cover art instead of app icon (Leaks credentials to Discord)

    // Audio Settings
    bool audio_equalizer_enable;
    bool audio_equalizer_followPitch;   // Have equalizer align to pitch adjustment
    int audio_equalizer_graphCount;
    configHandler_eqGraph_t* audio_equalizer_graph;
    bool audio_pitch_enable;
    double audio_pitch_cents;
    double audio_pitch_rate;
    bool audio_reverb_enable;
    double audio_reverb_wetDryMix;      // Reverb Wet/Dry Mix Percent

    // LV2 Audio Settings
    char* lv2_parax32_filter_name;      // LV2 LSP Para Equalizer x32 LR LV2 Name
    char* lv2_parax32_filter_type_left; // LV2 LSP Para Equalizer x32 LR Filter type left name
    char* lv2_parax32_filter_type_right;
    char* lv2_parax32_gain_left;
    char* lv2_parax32_gain_right;
    char* lv2_parax32_quality_left;
    char* lv2_parax32_quality_right;
    char* lv2_parax32_frequency_left;
    char* lv2_parax32_frequency_right;
    char* lv2_reverb_filter_name;       // LV2 Calf Reverb LV2 Name
} configHandler_config_t;

int configHandler_Read(configHandler_config_t** config);
void configHandler_Free(configHandler_config_t** config);

#endif

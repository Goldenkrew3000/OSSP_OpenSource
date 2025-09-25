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
} configHandler_config_t;

int configHandler_Read(configHandler_config_t** config);
void configHandler_Free(configHandler_config_t** config);

#endif

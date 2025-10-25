/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Gstreamer Handler
 */

#include <stdio.h>
#include <gst/gst.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "../configHandler.h"
#include "../discordrpc.h"
#include "../libopensubsonic/logger.h"
#include "../libopensubsonic/endpoint_getSong.h"
#include "../libopensubsonic/httpclient.h"
#include "player.h"

extern configHandler_config_t* configObj;
static int rc = 0;
GstElement *pipeline, *playbin, *filter_bin, *conv_in, *conv_out, *in_volume, *equalizer, *pitch, *reverb, *out_volume;
GstPad *sink_pad, *src_pad;
GstBus* bus;
guint bus_watch_id;
GMainLoop* loop;
bool isPlaying = false;

static void gst_playbin3_sourcesetup_callback(GstElement* playbin, GstElement* source, gpointer udata) {
    g_object_set(G_OBJECT(source), "user-agent", "OSSP/1.0 (avery@hojuix.org)", NULL);
}

static gboolean gst_bus_call(GstBus* bus, GstMessage* message, gpointer data) {
    GMainLoop* loop = (GMainLoop*)data;

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_EOS:
            logger_log_important(__func__, "[GBus] End of stream");
            gst_element_set_state(pipeline, GST_STATE_NULL);
            isPlaying = false;
            break;
        case GST_MESSAGE_BUFFERING: {
            gint percent = 0;
            gst_message_parse_buffering(message, &percent);
            printf("Buffering (%d%%)...\n", (int)percent);
            break;
        }
        case GST_MESSAGE_ERROR: {
            gchar* debug;
            GError* error;
            gst_message_parse_error(message, &error, &debug);
            printf("Gstreamer Error: %s\n", error);
            g_error_free(error);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
            printf("State changed\n");
            break;
        case GST_MESSAGE_NEW_CLOCK:
            //
            break;
        case GST_MESSAGE_LATENCY:
            //
            break;
        case GST_MESSAGE_STREAM_START:
            //
            break;
        case GST_MESSAGE_ELEMENT:
            //
            break;
        case GST_MESSAGE_ASYNC_DONE:
            //
            break;
        case GST_MESSAGE_STREAM_STATUS:
            //
            break;
        case GST_MESSAGE_STREAMS_SELECTED:
            //
            break;
        case GST_MESSAGE_STREAM_COLLECTION:
            //
            break;
        case GST_MESSAGE_DURATION_CHANGED:
            //
            break;
        case GST_MESSAGE_TAG:
            // Unused
            break;
        default:
            printf("Unknown Message. Type %ld\n", GST_MESSAGE_TYPE(message));
            break;
    }

    return TRUE;
}

void* OSSPlayer_GMainLoop(void*) {
    logger_log_important(__func__, "GMainLoop thread running.");
    // This is needed for the Gstreamer bus to work, but it hangs the thread
    g_main_loop_run(loop);
}

void* OSSPlayer_ThrdInit(void*) {
    // Player init function for pthread entry
    logger_log_important(__func__, "Player thread running.");
    OSSPlayer_GstInit();

    // Launch GMainLoop thread
    pthread_t pthr_gml;
    pthread_create(&pthr_gml, NULL, OSSPlayer_GMainLoop, NULL);

    // Poll play queue for new items to play
    while (true) { // TODO use global bool instead
        if (internal_OSSPQ_GetItemCount() != 0 && isPlaying == false) {
            printf("IS VALID\n");
            // Player is not playing and a song is in the song queue
            OSSPQ_SongStruct* pq = OSSPlayer_QueuePopFront();
            if (pq == NULL) {
                printf("FUCK\n");
                // TODO: this
            }

            char* id = strdup(pq->id);
            //free(pq);

            // NOTE: Using a few strdup()'s because the cleanup/deinit functions perform free's and to avoid UAFs/Double frees

            // Fetch song information
            opensubsonic_httpClient_URL_t* song_url = malloc(sizeof(opensubsonic_httpClient_URL_t));
            opensubsonic_httpClient_URL_prepare(&song_url);
            song_url->endpoint = OPENSUBSONIC_ENDPOINT_GETSONG;
            song_url->id = strdup(id);
            opensubsonic_httpClient_formUrl(&song_url);
            opensubsonic_getSong_struct* songStruct;
            opensubsonic_httpClient_fetchResponse(&song_url, (void**)&songStruct);

            // Generate the cover art URL
            opensubsonic_httpClient_URL_t* coverart_url = malloc(sizeof(opensubsonic_httpClient_URL_t));
            opensubsonic_httpClient_URL_prepare(&coverart_url);
            coverart_url->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
            coverart_url->id = strdup(id);
            opensubsonic_httpClient_formUrl(&coverart_url);

            // Update Discord RPC
            discordrpc_data* discordrpc = NULL;
            discordrpc_struct_init(&discordrpc);
            discordrpc->state = DISCORDRPC_STATE_PLAYING;
            discordrpc->songTitle = strdup(songStruct->title);
            discordrpc->songArtist = strdup(songStruct->artist);
            discordrpc->coverArtUrl = strdup(coverart_url->formedUrl);
            discordrpc_update(&discordrpc);
            discordrpc_struct_deinit(&discordrpc);

            opensubsonic_httpClient_URL_cleanup(&coverart_url);

            opensubsonic_getSong_struct_free(&songStruct);
            opensubsonic_httpClient_URL_cleanup(&song_url);

            // Create stream URL
            opensubsonic_httpClient_URL_t* stream_url = malloc(sizeof(opensubsonic_httpClient_URL_t));
            opensubsonic_httpClient_URL_prepare(&stream_url);
            stream_url->endpoint = OPENSUBSONIC_ENDPOINT_STREAM;
            stream_url->id = id;
            opensubsonic_httpClient_formUrl(&stream_url);

            g_object_set(playbin, "uri", stream_url->formedUrl, NULL);
            isPlaying = true;
            gst_element_set_state(pipeline, GST_STATE_PLAYING);
        }
        usleep(200 * 1000);
    }
}

int OSSPlayer_GstInit() {
    printf("[OSSP] Initializing Gstreamer...\n");

    // Initialize gstreamer
    gst_init(NULL, NULL);
    loop = g_main_loop_new(NULL, FALSE);

    // Create base pipeline elements
    pipeline = gst_pipeline_new("pipeline");
    playbin = gst_element_factory_make("playbin3", "player");
    // TODO: Fix erroring
    if (!pipeline) {
        logger_log_error(__func__, "Could not initialize pipeline.");
    }
    if (!playbin) {
        logger_log_error(__func__, "Could not initialize playbin3");
    }

    // Add message handler
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    // TODO: Check bus is made properly
    bus_watch_id = gst_bus_add_watch(bus, gst_bus_call, loop);
    gst_object_unref(bus);


    filter_bin = gst_bin_new("filter-bin");
    conv_in = gst_element_factory_make("audioconvert", "convert-in");
    conv_out = gst_element_factory_make("audioconvert", "convert-out");
    // TODO: Check creation

    // Create configuration defined elements
    in_volume = gst_element_factory_make("volume", "in-volume");
    if (configObj->audio_equalizer_enable) {
        // LSP Para x32 LR Equalizer
        equalizer = gst_element_factory_make(configObj->lv2_parax32_filter_name, "equalizer");
    }
    if (configObj->audio_pitch_enable) {
        // Soundtouch Pitch
        pitch = gst_element_factory_make("pitch", "pitch");
    }
    if (configObj->audio_reverb_enable) {
        // Calf Studio Plugins Reverb
        reverb = gst_element_factory_make(configObj->lv2_reverb_filter_name, "reverb");
    }
    out_volume = gst_element_factory_make("volume", "out-volume");
    // TODO: Make better error messages for here, and exit out early
    if (!equalizer) {
        logger_log_error(__func__, "Could not initialize equalizer.");
    }
    if (!pitch) {
        logger_log_error(__func__, "Could not initialize pitch.");
    }
    if (!reverb) {
        logger_log_error(__func__, "Could not initialize reverb.");
    }

    // Add and link elements to the filter bin
    // TODO: Check creation and dynamic as per config
    gst_bin_add_many(GST_BIN(filter_bin), conv_in, in_volume, equalizer, pitch, out_volume, conv_out, NULL);
    gst_element_link_many(conv_in, in_volume, equalizer, pitch, out_volume, conv_out, NULL);
    sink_pad = gst_element_get_static_pad(conv_in, "sink");
    src_pad = gst_element_get_static_pad(conv_out, "src");
    gst_element_add_pad(filter_bin, gst_ghost_pad_new("sink", sink_pad));
    gst_element_add_pad(filter_bin, gst_ghost_pad_new("src", src_pad));
    gst_object_unref(sink_pad);
    gst_object_unref(src_pad);

    // Setup playbin3 (Configure audio plugins and set user agent)
    g_object_set(playbin, "audio-filter", filter_bin, NULL);
    g_signal_connect(playbin, "source-setup", G_CALLBACK(gst_playbin3_sourcesetup_callback), NULL);

    // Add playbin3 to the pipeline
    gst_bin_add(GST_BIN(pipeline), playbin);

    // Initialize in-volume (Volume before the audio reaches the plugins)
    g_object_set(in_volume, "volume", 0.175, NULL);

    // Initialize out-volume (Volume after the audio plugins)
    g_object_set(out_volume, "volume", 1.00, NULL);

    // Initialize equalizer
    if (configObj->audio_equalizer_enable) {
        // Dynamically append settings to the equalizer to match the config file
        for (int i = 0; i < configObj->audio_equalizer_graphCount; i++) {
            char* ftl_name = NULL;
            char* ftr_name = NULL;
            char* gl_name = NULL;
            char* gr_name = NULL;
            char* ql_name = NULL;
            char* qr_name = NULL;
            char* fl_name = NULL;
            char* fr_name = NULL;

            asprintf(&ftl_name, "%s%d", configObj->lv2_parax32_filter_type_left, i);
            asprintf(&ftr_name, "%s%d", configObj->lv2_parax32_filter_type_right, i);
            asprintf(&gl_name, "%s%d", configObj->lv2_parax32_gain_left, i);
            asprintf(&gr_name, "%s%d", configObj->lv2_parax32_gain_right, i);
            asprintf(&ql_name, "%s%d", configObj->lv2_parax32_quality_left, i);
            asprintf(&qr_name, "%s%d", configObj->lv2_parax32_quality_right, i);
            asprintf(&fl_name, "%s%d", configObj->lv2_parax32_frequency_left, i);
            asprintf(&fr_name, "%s%d", configObj->lv2_parax32_frequency_right, i);

            g_object_set(equalizer, ftl_name, 1, NULL);
            g_object_set(equalizer, ftr_name, 1, NULL);

            // NOTE: Making an extra variable here to avoid nesting a function within a function
            float gain = (float)configObj->audio_equalizer_graph[i].gain;
            gain = OSSPlayer_DbLinMul(gain);
            g_object_set(equalizer, gl_name, gain, NULL);
            g_object_set(equalizer, gr_name, gain, NULL);

            g_object_set(equalizer, ql_name, 4.36, NULL);
            g_object_set(equalizer, qr_name, 4.36, NULL);

            // NOTE: Same function nesting mitigation here
            if (configObj->audio_equalizer_followPitch) {
                // Adjust equalizer frequency to match pitch adjustment
                // TODO: Should I also check if pitch is enabled, or just if pitch follow is enabled??
                // TODO: Also check that freq following is working properly as per swift version
                float freq = (float)configObj->audio_equalizer_graph[i].frequency;
                float semitone = (float)configObj->audio_pitch_cents / 100.0;
                freq = OSSPlayer_PitchFollow(freq, semitone);
                printf("EQ band %d - F: %.2f(Fp) / G: %.2f / Q: 4.36\n", i + 1, freq, gain);
                g_object_set(equalizer, fl_name, freq, NULL);
                g_object_set(equalizer, fr_name, freq, NULL);
            } else {
                printf("EQ band %d - F: %.2f(Nfp) / G: %.2f / Q: 4.36\n", i + 1, (float)configObj->audio_equalizer_graph[i].frequency, gain);
                g_object_set(equalizer, fl_name, (float)configObj->audio_equalizer_graph[i].frequency, NULL);
                g_object_set(equalizer, fr_name, (float)configObj->audio_equalizer_graph[i].frequency, NULL);
            }

            free(ftl_name);
            free(ftr_name);
            free(gl_name);
            free(gr_name);
            free(ql_name);
            free(qr_name);
            free(fl_name);
            free(fr_name);
        }

        g_object_set(equalizer, "enabled", true, NULL);
    }

    // Initialize pitch
    if (configObj->audio_pitch_enable) {
        float scaleFactor = OSSPlayer_CentsToPSF(configObj->audio_pitch_cents);
        printf("Pitch Cents: %.2f, Scale factor: %.6f\n", configObj->audio_pitch_cents, scaleFactor);
        g_object_set(pitch, "pitch", scaleFactor, NULL);
    }

    // Initialize reverb
}

int OSSPlayer_GstDeInit() {
    //
}

/*
 * Player Queue Control Functions
 */
int OSSPlayer_QueueAppend(char* title, char* artist, char* id, long duration) {
    // Call to C++ function
    // Note: I would receive a song struct instead of individual elements, but it would significantly slow down the GUI
    internal_OSSPQ_AppendToEnd(title, artist, id, duration);
}

OSSPQ_SongStruct* OSSPlayer_QueuePopFront() {
    // Call to C++ function

    OSSPQ_SongStruct* songObject = internal_OSSPQ_PopFromFront();

    if (songObject == NULL) {
        // Queue is empty TODO
        printf("FUCKFUCKFUCK\n");
    }
    return songObject;
}

/*
 * Gstreamer Element Control Functions
 */
// TODO: Consolidate volume functions?
float OSSPlayer_GstECont_InVolume_Get() {
    gdouble vol;
    g_object_get(in_volume, "volume", &vol, NULL);
    return (float)vol;
}

void OSSPlayer_GstECont_InVolume_set(float val) {
    g_object_set(in_volume, "volume", val, NULL);
}

float OSSPlayer_GstECont_OutVolume_Get() {
    gdouble vol;
    g_object_get(out_volume, "volume", &vol, NULL);
    return (float)vol;
}

void OSSPlayer_GstECont_OutVolume_set(float val) {
    g_object_set(out_volume, "volume", val, NULL);
}

float OSSPlayer_GstECont_Pitch_Get() {
    //
}

void OSSPlayer_GstECont_Pitch_Set(float cents) {
    float psf = OSSPlayer_CentsToPSF(cents);
    g_object_set(pitch, "pitch", psf, NULL);
}

/*
 * Utility Functions
 */
float OSSPlayer_DbLinMul(float db) {
    // Convert dB to Linear Multiplier
    return pow(10.0, db / 20.0);
}

float OSSPlayer_PitchFollow(float freq, float semitone) {
    // Calculate new EQ frequency from semitone adjustment
    return freq * pow(2.0, semitone / 12.0);
}

float OSSPlayer_CentsToPSF(float cents) {
    // Convert Cents to a Pitch Scale Factor
    float semitone = cents / 100.0;
    return pow(2, (semitone / 12.0f));
}

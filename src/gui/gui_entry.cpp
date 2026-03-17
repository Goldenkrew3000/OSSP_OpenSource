/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Debug / Prototype graphical interface
 */

/*
 * THIS IS SPECIFICALLY THE DEVELOPMENT INTERFACE
 * IT IS HORRIFICIALLY UNOPTIMIZED BUT IT WORKS
 */

#include "../external/imgui/imgui.h"
#include "../external/imgui/backends/imgui_impl_sdl2.h"
#include "../external/imgui/backends/imgui_impl_opengl2.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "gui_entry.hpp"
#include "../configHandler.h"
#include "../libopensubsonic/httpclient.h"
#include "../libopensubsonic/endpoint_getStarred.h"
#include "../libopensubsonic/endpoint_getAlbum.h"
#include "../player/player.h"
#include "../libopensubsonic/endpoint_getInternetRadioStations.h"

extern configHandler_config_t* configObj;
bool bLikedSongsShow = false;
bool bAudioSettingsShow = false;
bool bPlayQueueShow = false;
bool bShowRadioStations = false;
bool bShowNowPlaying = false;
bool bShowLikedAlbums = false;
bool bShowLocalSongs = false;
void showLikedSongs();
void showAudioSettings();
void showPlayQueue();
void showRadioStations();
void showNowPlaying();
void showLikedAlbums();
void showLocalSongs();

int gui_entry() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("SDL could not be initialized: %s\n", SDL_GetError());
        return 1;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("OSSP v0.3a", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr) {
        printf("SDL could not create window: %s\n", SDL_GetError());
        return 1;
    }

    // Create GL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Vsync

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    // Scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Setup platform/renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    // START START START
    ImVec4 background_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    bool done = false;

    while (!done) {
        // Poll events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done = true;
            }
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start new frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("OSSP v0.3a");

            ImGui::Text("Connected to server at %s://%s", configObj->opensubsonic_protocol, configObj->opensubsonic_server);
            if (ImGui::Button("Liked Songs")) {
                printf("Liked songs button pressed\n");
                bLikedSongsShow = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Audio Settings")) {
                bAudioSettingsShow = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Play Queue")) {
                bPlayQueueShow = true;
            }

            if (ImGui::Button("Radio")) {
                bShowRadioStations = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Now Playing")) {
                bShowNowPlaying = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Liked Albums")) {
                bShowLikedAlbums = true;
            }

            if (ImGui::Button("Local Songs")) {
                bShowLocalSongs = true;
            }

            ImGui::End();
        }

        if (bLikedSongsShow) {
            showLikedSongs();
        }

        if (bAudioSettingsShow) {
            showAudioSettings();
        }

        if (bPlayQueueShow) {
            showPlayQueue();
        }

        if (bShowRadioStations) {
            showRadioStations();
        }

        if (bShowNowPlaying) {
            showNowPlaying();
        }

        if (bShowLikedAlbums) {
            showLikedAlbums();
        }

        if (bShowLocalSongs) {
            showLocalSongs();
        }

        // Render
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(background_color.x * background_color.w, background_color.y * background_color.w, background_color.z * background_color.w, background_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}



bool haveLikedSongsInfo = false;
opensubsonic_getStarred_struct* starredStruct;
opensubsonic_httpClient_URL_t* starredUrl;

void getLikedSongsInfo() {
    // Pull liked songs
    starredUrl = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&starredUrl);
    starredUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETSTARRED;
    opensubsonic_httpClient_formUrl(&starredUrl);
    opensubsonic_httpClient_fetchResponse(&starredUrl, (void**)&starredStruct);

    if (starredStruct->errorCode != 0) {
        // Error occured
    }

    haveLikedSongsInfo = true;
}

void showLikedSongs() {
    if (!haveLikedSongsInfo) { getLikedSongsInfo(); }

    ImGui::Begin("Liked Songs");

    ImGui::Text("Liked Songs");

    if (ImGui::Button("Close")) {
        bLikedSongsShow = false;
    }

    if (ImGui::Button("Refresh")) {
        opensubsonic_getStarred_struct_free(&starredStruct);
        opensubsonic_httpClient_URL_cleanup(&starredUrl);
        haveLikedSongsInfo = false;
    }

    static int selectedSong = -1;
    if (haveLikedSongsInfo) {
        if (ImGui::BeginChild("Liked Songs", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < starredStruct->songCount; i++) {
                if (ImGui::Selectable(starredStruct->songs[i].title, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    if (selectedSong != -1) {
        // Form URL
        opensubsonic_httpClient_URL_t* song_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
        opensubsonic_httpClient_URL_prepare(&song_url);
        song_url->endpoint = OPENSUBSONIC_ENDPOINT_STREAM;
        song_url->id = strdup(starredStruct->songs[selectedSong].id);
        opensubsonic_httpClient_formUrl(&song_url);

        opensubsonic_httpClient_URL_t* coverart_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
        opensubsonic_httpClient_URL_prepare(&coverart_url);
        coverart_url->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
        coverart_url->id = strdup(starredStruct->songs[selectedSong].coverArt);
        opensubsonic_httpClient_formUrl(&coverart_url);
        
        OSSPQ_AppendToEnd(starredStruct->songs[selectedSong].title,
                          starredStruct->songs[selectedSong].album,
                          starredStruct->songs[selectedSong].artist,
                          starredStruct->songs[selectedSong].id,
                          song_url->formedUrl,
                          coverart_url->formedUrl,
                          starredStruct->songs[selectedSong].duration,
                          OSSPQ_MODE_OPENSUBSONIC);
        
        opensubsonic_httpClient_URL_cleanup(&song_url);
        opensubsonic_httpClient_URL_cleanup(&coverart_url);
        selectedSong = -1;
    }

    ImGui::End();
}

float in_volume_val = 0;
float out_volume_val = 0;
float pitch_val = 0;
bool hasInVolumeFirstRun = false;
void showAudioSettings() {
    ImGui::Begin("Audio Settings");

    if (!hasInVolumeFirstRun) {
        in_volume_val = OSSPlayer_GstECont_InVolume_Get();
        out_volume_val = OSSPlayer_GstECont_OutVolume_Get();
        pitch_val = configObj->audio_pitch_cents / 100.0f; // Cents to semitones
        hasInVolumeFirstRun = true;
    }

    ImGui::Text("In Vol / Out Vol");

    // Idk what that field is, styling?, Size, Storage, Low, High
    if (ImGui::VSliderFloat("##invol", ImVec2(35, 160), &in_volume_val, 0.0f, 1.0f)) {
        // Data has changed
        OSSPlayer_GstECont_InVolume_set(in_volume_val);
    }

    ImGui::SameLine();

    if (ImGui::VSliderFloat("##outvol", ImVec2(35, 160), &out_volume_val, 0.0f, 1.0f)) {
        OSSPlayer_GstECont_OutVolume_set(out_volume_val);
    }

    ImGui::SameLine();

    if (ImGui::VSliderFloat("##pitch", ImVec2(35, 160), &pitch_val, -6.00f, 6.00f)) {
        OSSPlayer_GstECont_Pitch_Set(pitch_val * 100.0f); // Convert semitones to cents
    }

    if(ImGui::Button("Skip")) {
        OSSPlayer_GstECont_Playbin3_Stop();
    }

    ImGui::End();
}

// TODO: go through abstraction
#include "../player/playQueue.hpp"

void showPlayQueue() {
    ImGui::Begin("Play Queue");

    static int selectedSong = -1;
            if (ImGui::BeginChild("Play Queue", ImVec2(0, 200), ImGuiChildFlags_Border)) {
                for (int i = 0; i < internal_OSSPQ_GetItemCount(); i++) {
                    if (ImGui::Selectable(internal_OSSPQ_GetTitleAtIndex(i), selectedSong == i)) {
                        selectedSong = i;
                    }
                }
            ImGui::EndChild();
            }

    ImGui::End();
}



bool haveRadioStations = false;
opensubsonic_getInternetRadioStations_struct* irsStruct;
opensubsonic_httpClient_URL_t* radioUrl;

void getRadioStations() {
    radioUrl = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&radioUrl);
    radioUrl->endpoint = OPENSUBSONIC_ENDPOINT_GETINTERNETRADIOSTATIONS;
    opensubsonic_httpClient_formUrl(&radioUrl);
    opensubsonic_httpClient_fetchResponse(&radioUrl, (void**)&irsStruct);

    if (irsStruct->errorCode != 0) {
        // Error happened
    }

    haveRadioStations = true;
}

void showRadioStations() {
    if (!haveRadioStations) { getRadioStations(); }

    ImGui::Begin("Radio Stations");

    static int selectedSong = -1;
    if (haveRadioStations) {
        if (ImGui::BeginChild("Radio Stations", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < irsStruct->radioStationCount; i++) {
                if (ImGui::Selectable(irsStruct->radioStations[i].name, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    
    if (selectedSong != -1) {
        //OSSPlayer_QueueAppend_Radio(irsStruct->radioStations[selectedSong].name,
        //                      irsStruct->radioStations[selectedSong].id,
        //                      irsStruct->radioStations[selectedSong].streamUrl);
        selectedSong = -1;
    }

    ImGui::End();
}

void showNowPlaying() {
    ImGui::Begin("Now Playing");

    /*
     * Okay so I need:
     *  - Current and final position of song
     *  - Play, Pause, Next buttons (Needs DiscordRPC expansion)
     */

    ImGui::End();
}


void showAlbum(char* id);

int likedAlbumsSelectedSong = -1;
void showLikedAlbums() {
    // /getStarred is all of the liked info, not just songs
    if (!haveLikedSongsInfo) { getLikedSongsInfo(); }

    ImGui::Begin("Liked Albums");

    ImGui::Text("Liked Albums");

    if (ImGui::Button("Close")) {
        bShowLikedAlbums = false;
    }

    if (ImGui::Button("Refresh")) {
        opensubsonic_getStarred_struct_free(&starredStruct);
        opensubsonic_httpClient_URL_cleanup(&starredUrl);
        haveLikedSongsInfo = false;
    }

    if (haveLikedSongsInfo) {
        if (ImGui::BeginChild("Liked Albums", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < starredStruct->albumCount; i++) {
                if (ImGui::Selectable(starredStruct->albums[i].title, likedAlbumsSelectedSong == i)) {
                    likedAlbumsSelectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    if (likedAlbumsSelectedSong != -1) {
        showAlbum(starredStruct->albums[likedAlbumsSelectedSong].id);
    }

    ImGui::End();
}

bool hasAlbum = false;
opensubsonic_getAlbum_struct* getAlbumStruct;
void getAlbum(char* id) {
    opensubsonic_httpClient_URL_t* url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
    opensubsonic_httpClient_URL_prepare(&url);
    url->endpoint = OPENSUBSONIC_ENDPOINT_GETALBUM;
    url->id = strdup(id);
    opensubsonic_httpClient_formUrl(&url);
    
    opensubsonic_httpClient_fetchResponse(&url, (void**)&getAlbumStruct);
    
    //opensubsonic_getAlbum_struct_free(&getAlbumStruct);
    //opensubsonic_httpClient_URL_cleanup(&url);

    hasAlbum = true;
}

static int rand_int(int n) {
  int limit = RAND_MAX - RAND_MAX % n;
  int rnd;

  do {
    rnd = rand();
  } while (rnd >= limit);
  return rnd % n;
}

void shuffle(int *array, int n) {
  int i, j, tmp;

  for (i = n - 1; i > 0; i--) {
    j = rand_int(i + 1);
    tmp = array[j];
    array[j] = array[i];
    array[i] = tmp;
  }
}

void shuffleAlbum() {
    int n = getAlbumStruct->songCount;

    int arr[1] = { 0 };
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }

    shuffle(arr, n);

    for (int i = 0; i < n; i++) {
        //OSSPlayer_QueueAppend_Song(getAlbumStruct->songs[arr[i]].title,
         //                     getAlbumStruct->songs[arr[i]].artist,
        //                      getAlbumStruct->songs[arr[i]].id,
        //                      getAlbumStruct->songs[arr[i]].duration);
    }
}

#include <ctime>
void showAlbum(char* id) {
    ImGui::Begin("Album");

    if (!hasAlbum) { getAlbum(id); }

    ImGui::Text("Album");

    if (ImGui::Button("Close")) {
        likedAlbumsSelectedSong = -1;
        hasAlbum = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Play all")) {
        for (int i = 0; i < getAlbumStruct->songCount; i++) {
            /*
            // Form URL
            opensubsonic_getAlbum_struct*

            opensubsonic_httpClient_URL_t* song_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
            opensubsonic_httpClient_URL_prepare(&song_url);
            song_url->endpoint = OPENSUBSONIC_ENDPOINT_STREAM;
            song_url->id = strdup(starredStruct->songs[selectedSong].id);
            opensubsonic_httpClient_formUrl(&song_url);

            opensubsonic_httpClient_URL_t* coverart_url = (opensubsonic_httpClient_URL_t*)malloc(sizeof(opensubsonic_httpClient_URL_t));
            opensubsonic_httpClient_URL_prepare(&coverart_url);
            coverart_url->endpoint = OPENSUBSONIC_ENDPOINT_GETCOVERART;
            coverart_url->id = strdup(starredStruct->songs[selectedSong].coverArt);
            opensubsonic_httpClient_formUrl(&coverart_url);

            OSSPQ_AppendToEnd(starredStruct->songs[selectedSong].title,
                              starredStruct->songs[selectedSong].album,
                              starredStruct->songs[selectedSong].artist,
                              starredStruct->songs[selectedSong].id,
                              song_url->formedUrl,
                              coverart_url->formedUrl,
                              starredStruct->songs[selectedSong].duration,
                              OSSPQ_MODE_OPENSUBSONIC);

            opensubsonic_httpClient_URL_cleanup(&song_url);
            opensubsonic_httpClient_URL_cleanup(&coverart_url);
            selectedSong = -1;                   getAlbumStruct->songs[i].duration);
            */
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Shuffle")) {
        srand(time(NULL));
        shuffleAlbum();
    }

    static int selectedSong = -1;
    if (hasAlbum) {
        if (ImGui::BeginChild("Album", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < getAlbumStruct->songCount; i++) {
                if (ImGui::Selectable(getAlbumStruct->songs[i].title, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    ImGui::End();
}







#include "../localMusicHandler.hpp"

bool hasLocalSongs = false;
localMusicHandler_songReq_t* songReq;

void getLocalSongs() {
    songReq = localMusicHandler_test();
    hasLocalSongs = true;
}

void showLocalSongs() {
    

    ImGui::Begin("Local Songs");

    if (!hasLocalSongs) { getLocalSongs(); }

    ImGui::Text("Local Songs");

    static int selectedSong = -1;
    if (hasLocalSongs) {
        if (ImGui::BeginChild("LocalSongs", ImVec2(0, 200), ImGuiChildFlags_Border)) {
            for (int i = 0; i < songReq->songCount; i++) {
                if (ImGui::Selectable(songReq->songs[i].title, selectedSong == i)) {
                    selectedSong = i;
                }
            }
        ImGui::EndChild();
        }
    }

    if (selectedSong != -1) {
        // Treat it as radio station for testing
        char* newPath = NULL;
        asprintf(&newPath, "file://%s", songReq->songs[selectedSong].path);

        //OSSPlayer_QueueAppend_Radio(songReq->songs[selectedSong].title,
        //                      songReq->songs[selectedSong].uid,
        //                      newPath);
        OSSPQ_AppendToEnd(songReq->songs[selectedSong].title,
                          NULL,
                          NULL,
                          songReq->songs[selectedSong].uid,
                          newPath,
                          NULL,
                          0,
                          OSSPQ_MODE_LOCALFILE);
        selectedSong = -1;
    }

    ImGui::End();
}

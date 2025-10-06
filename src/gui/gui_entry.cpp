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

extern configHandler_config_t* configObj;
bool bLikedSongsShow = false;
void showLikedSongs();

int qt_gui_entry(int argc, char** argv) {
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

            ImGui::End();
        }

        if (bLikedSongsShow) {
            showLikedSongs();
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
    if (ImGui::BeginChild("Liked Songs", ImVec2(0, 200), ImGuiChildFlags_Border)) {
        for (int i = 0; i < starredStruct->songCount; i++) {
            if (ImGui::Selectable(starredStruct->songs[i].title, selectedSong == i)) {
                selectedSong = i;
            }
        }
        ImGui::EndChild();
    }

    if (selectedSong != -1) {
        printf("Song: %s (%s)\n", starredStruct->songs[selectedSong].title,
            starredStruct->songs[selectedSong].id);
        selectedSong = -1;
    }

    ImGui::End();
}

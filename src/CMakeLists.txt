cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ossp)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

add_subdirectory(external/discord-rpc)

add_executable(ossp MACOSX_BUNDLE
    main.c
    gui/gui_entry.cpp
    configHandler.c
    player/player.c
    player/playQueue.cpp
    libopensubsonic/crypto.c
    libopensubsonic/httpclient.c
    libopensubsonic/logger.c
    libopensubsonic/utils.c
    libopensubsonic/scrobble_lastFm.c
    libopensubsonic/scrobble_listenBrainz.c
    libopensubsonic/endpoint_getAlbum.c
    libopensubsonic/endpoint_getAlbumList.c
    libopensubsonic/endpoint_getArtist.c
    libopensubsonic/endpoint_getArtists.c
    libopensubsonic/endpoint_getLyricsBySongId.c
    libopensubsonic/endpoint_getPlaylist.c
    libopensubsonic/endpoint_getPlaylists.c
    libopensubsonic/endpoint_getSong.c
    libopensubsonic/endpoint_getStarred.c
    libopensubsonic/endpoint_ping.c
    libopensubsonic/endpoint_scrobble.c
    external/cJSON.c
    external/cJSON_Utils.c
    external/libcurl_uriescape.c
    external/md5.c
    external/imgui/imgui.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
    external/imgui/backends/imgui_impl_opengl2.cpp
)

set_target_properties(ossp PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "OSSP"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.hojuix.ossp"
)

include_directories(${GSTREAMER_INCLUDE_DIRS})

target_link_libraries(ossp PRIVATE OpenSSL::SSL OpenSSL::Crypto CURL::libcurl SDL2::SDL2 ${OPENGL_LIBRARIES} discord-rpc ${GSTREAMER_LIBRARIES})

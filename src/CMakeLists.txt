cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ossp)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(ossp
    main.c
    gui/gui_entry.cpp
    gui/window.cpp
    gui/songWidget.cpp
    configHandler.c
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
)

target_link_libraries(ossp PRIVATE OpenSSL::SSL OpenSSL::Crypto CURL::libcurl Qt6::Core Qt6::Widgets)

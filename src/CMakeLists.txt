cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ossp)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

add_subdirectory(external/discord-rpc)

add_executable(ossp MACOSX_BUNDLE
    main.c
    configHandler.c
    discordrpc.c
    localRadioDBHandler.c
    localMusicHandler.cpp
    socket/socket.c
    socket/socketActions.c
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
    libopensubsonic/endpoint_getInternetRadioStations.c
    external/cJSON.c
    external/cJSON_Utils.c
    external/libcurl_uriescape.c
    external/md5.c
    external/sqlite3/sqlite3.c
)

# Use macOS .app format (Required for Discord RPC)
set_target_properties(ossp PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "OSSP"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.hojuix.ossp"
)

if(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    message("--- Detected NetBSD ---")
    target_link_directories(ossp PRIVATE /usr/pkg/lib/ffmpeg8)
    include_directories(${GSTREAMER_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIRS} /usr/X11R7/include /usr/pkg/include/ffmpeg8)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message("--- Detected macOS ---")
    target_link_directories(ossp PRIVATE /opt/ossp/lib)
    pkg_check_modules(AVFORMAT REQUIRED libavformat)
    pkg_check_modules(AVUTIL REQUIRED libavutil)
    pkg_check_modules(BZ2 REQUIRED bzip2)
    include_directories(${GSTREAMER_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIRS})
else()
    message("--- Detected generic operating system ---")
    pkg_check_modules(AVFORMAT REQUIRED libavformat)
    pkg_check_modules(AVUTIL REQUIRED libavutil)
    include_directories(${GSTREAMER_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIRS})
endif()

target_link_libraries(ossp PRIVATE OpenSSL::SSL OpenSSL::Crypto CURL::libcurl discord-rpc ${BZ2_LIBRARIES} ${GSTREAMER_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVUTIL_LIBRARIES} avformat avutil)

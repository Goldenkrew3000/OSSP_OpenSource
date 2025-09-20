egcc main.c \
    configHandler.c \
    libopensubsonic/httpclient.c \
    external/cJSON.c \
    external/md5.c \
    libopensubsonic/utils.c \
    libopensubsonic/logger.c \
    libopensubsonic/crypto.c \
    libopensubsonic/endpoint_ping.c \
    libopensubsonic/endpoint_getAlbum.c \
    libopensubsonic/endpoint_getSong.c \
    libopensubsonic/endpoint_getPlaylists.c \
    libopensubsonic/endpoint_getPlaylist.c \
    libopensubsonic/endpoint_getArtists.c \
    libopensubsonic/endpoint_getArtist.c \
    libopensubsonic/endpoint_getLyricsBySongId.c \
    libopensubsonic/endpoint_getAlbumList.c \
    libopensubsonic/endpoint_getStarred.c \
    libopensubsonic/endpoint_scrobble.c \
    -o main -I/usr/local/include -L/usr/local/lib -lcurl

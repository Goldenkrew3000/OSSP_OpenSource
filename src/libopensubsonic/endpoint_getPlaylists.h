#ifndef _ENDPOINT_GETPLAYLISTS_H
#define _ENDPOINT_GETPLAYLISTS_H

typedef struct {
    char* id;           // Album ID
    char* name;         // Album name
    int songCount;      // Number of songs in the album
    long duration;      // Duration of the album in seconds
    // 'public' doesn't seem useful here
    char* owner;        // Username of the owner of the album
    // 'created' and 'changed' do not seem useful here either
    char* coverArt;     // Cover art ID of the album
} opensubsonic_getPlaylists_playlist_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    int playlistCount;
    opensubsonic_getPlaylists_playlist_struct* playlists;
} opensubsonic_getPlaylists_struct;

int opensubsonic_getPlaylists_parse(char* data, opensubsonic_getPlaylists_struct** getPlaylistsStruct);
void opensubsonic_getPlaylists_struct_free(opensubsonic_getPlaylists_struct** getPlaylistsStruct);

#endif

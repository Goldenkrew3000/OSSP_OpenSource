#ifndef _ENDPOINT_GETPLAYLIST_H
#define _ENDPOINT_GETPLAYLIST_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    char* id;
    char* parent;
    char* title;
    // 'isDir', 'isVideo', and 'type' are not needed
    char* albumId;
    char* album;        // Album Name
    char* artistId;
    char* artist;       // Artist Name
    char* coverArt;     // Cover Art ID
    long duration;      // Duration (seconds)
    // Do not completely rely on the next 4 values if you can detect them manually
    int bitRate;
    int bitDepth;
    long samplingRate;
    int channelCount;
    int userRating;
    // Not sure what 'averageRating' is used for
    int track;
    int year;           // Year released
    char* genre;
    long size;          // Size in bytes
    int discNumber;
    // 'suffix', 'contentType', and 'path' not needed
} opensubsonic_getPlaylist_songs_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    char* id;
    char* name;
    char* owner;
    bool isPublic;      // 'public'
    char* created;
    char* changed;
    long duration;
    int songCount; // Counts are not reliable from navidrome, counted manually
    opensubsonic_getPlaylist_songs_struct* songs;
} opensubsonic_getPlaylist_struct;

int opensubsonic_getPlaylist_parse(char* data, opensubsonic_getPlaylist_struct** getPlaylistStruct);
void opensubsonic_getPlaylist_struct_free(opensubsonic_getPlaylist_struct** getPlaylistStruct);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _ENDPOINT_GETPLAYLIST_H

#ifndef _ENDPOINT_GETALBUM_H
#define _ENDPOINT_GETALBUM_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    char* id;
    char* parent;
    char* title;
    // 'isDir', 'isVideo', 'type' excluded
    char* albumId;
    char* album;
    char* artistId;
    char* artist;
    char* coverArt;
    long duration;
    int bitRate;
    int bitDepth;
    long samplingRate;
    int channelCount;
    int track; // Use songCount index instead
    int year;
    char* genre;
    long size;
    int discNumber;
    // 'suffix', 'contentType', 'path' excluded
} opensubsonic_getAlbum_song_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    char* id;
    char* parent;
    char* album;
    char* title;
    char* name;
    // 'isDir' excluded
    char* coverArt;
    char* created;
    long duration;
    int playCount;
    char* artistId;
    char* artist;
    int year;
    char* genre;
    int songCount; // Counted, not retrieved from JSON
    opensubsonic_getAlbum_song_struct* songs;
} opensubsonic_getAlbum_struct;

int opensubsonic_getAlbum_parse(char* data, opensubsonic_getAlbum_struct** getAlbumStruct);
void opensubsonic_getAlbum_struct_free(opensubsonic_getAlbum_struct** getAlbumStruct);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _ENDPOINT_GETALBUM_H

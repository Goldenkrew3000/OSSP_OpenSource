#ifndef _ENDPOINT_GETSTARRED_H
#define _ENDPOINT_GETSTARRED_H

typedef struct {
    char* id;
    char* name;
    char* coverArt;
    char* starred;
} opensubsonic_getStarred_artist_struct;

typedef struct {
    char* id;
    char* parent;
    char* album;
    char* title;
    char* name;
    // 'isDir' excluded
    char* coverArt;
    int songCount; // Do not rely on this
    char* created;
    long duration;
    int playCount;
    char* artistId;
    char* artist;
    int year;
    char* genre;
} opensubsonic_getStarred_album_struct;

typedef struct {
    char* id;
    char* parent;
    // 'isDir' excluded
    char* title;
    char* album;
    char* artist;
    int track;
    int year;
    char* coverArt;
    long size;
    // 'contentType', 'suffix' excluded
    char* starred;
    long duration;
    int bitRate;
    int bitDepth;
    long samplingRate;
    int channelCount;
    // 'path' excluded
    int playCount;
    int discNumber;
    char* created;
    char* albumId;
    char* artistId;
    // 'type', 'isVideo' excluded
} opensubsonic_getStarred_song_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    int artistCount;
    int albumCount;
    int songCount;
    opensubsonic_getStarred_artist_struct* artists;
    opensubsonic_getStarred_album_struct* albums;
    opensubsonic_getStarred_song_struct* songs;
} opensubsonic_getStarred_struct;

int opensubsonic_getStarred_parse(char* data, opensubsonic_getStarred_struct** getStarredStruct);
void opensubsonic_getStarred_struct_free(opensubsonic_getStarred_struct** getStarredStruct);

#endif

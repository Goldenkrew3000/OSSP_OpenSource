#ifndef _ENDPOINT_GETALBUMLIST_H
#define _ENDPOINT_GETALBUMLIST_H

typedef struct {
    char* id;
    char* parent;
    char* album;
    char* title;
    char* name;
    // 'isDir' excluded
    char* coverArt;
    int songCount;          // Only use as a guideline, count manually when possible
    char* created;
    long duration;
    int playCount;
    char* artistId;
    char* artist;
    int year;
    char* genre;
} opensubsonic_getAlbumList_album_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    int albumCount;         // Album count (Counted from array, NOT from JSON)
    opensubsonic_getAlbumList_album_struct* albums;
} opensubsonic_getAlbumList_struct;

int opensubsonic_getAlbumList_parse(char* data, opensubsonic_getAlbumList_struct** getAlbumListStruct);
void opensubsonic_getAlbumList_struct_free(opensubsonic_getAlbumList_struct** getAlbumListStruct);

#endif

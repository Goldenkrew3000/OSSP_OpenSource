#ifndef _ENDPOINT_GETARTIST_H
#define _ENDPOINT_GETARTIST_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    char* id;           // Album ID
    char* parent;       // Parent ID
    char* title;        // Album Title
    char* name;         // Album Name
    // 'isDir' excluded
    char* coverArt;     // Cover art ID
    int songCount;
    char* created;      // Time created
    long duration;      // Duration of the album in seconds
    int playCount;
    char* artistId;     // Artist ID
    char* artist;       // Artist Name
    int year;           // Year released
    char* genre;
    int userRating;
    char* starred;      // Time starred
} opensubsonic_getArtist_album_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    char* artistId;         // Artist ID
    char* artistName;       // Artist name
    char* coverArt;         // Artist cover art ID
    int userRating;
    char* artistImageUrl;
    char* starred;          // Time starred
    // 'musicBrainzId', 'sorted', 'roles' excluded
    int albumCount;         // Album count (Counted from array, NOT from JSON)
    opensubsonic_getArtist_album_struct* albums;
} opensubsonic_getArtist_struct;

int opensubsonic_getArtist_parse(char* data, opensubsonic_getArtist_struct** getArtistStruct);
void opensubsonic_getArtist_struct_free(opensubsonic_getArtist_struct** getArtistStruct);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _ENDPOINT_GETARTIST_H

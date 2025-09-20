#ifndef _ENDPOINT_GETARTISTS_H
#define _ENDPOINT_GETARTISTS_H

typedef struct {
    char* id;
    char* name;
    char* coverArt;
    int albumCount;
} opensubsonic_getArtists_artist_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    int artistCount;
    opensubsonic_getArtists_artist_struct* artists;
} opensubsonic_getArtists_struct;

int opensubsonic_getArtists_parse(char* data, opensubsonic_getArtists_struct** getArtistsStruct);
void opensubsonic_getArtists_struct_free(opensubsonic_getArtists_struct** getArtistsStruct);

#endif

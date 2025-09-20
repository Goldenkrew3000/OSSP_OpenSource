#ifndef _ENDPOINT_GETLYRICSBYSONGID_H
#define _ENDPOINT_GETLYRICSBYSONGID_H

typedef struct {
    char* data;
    long offset;
} opensubsonic_getLyricsBySongId_lyric_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    char* displayArtist;
    char* displayTitle;
    int lyricsAmount;
    opensubsonic_getLyricsBySongId_lyric_struct* lyrics;
} opensubsonic_getLyricsBySongId_struct;

int opensubsonic_getLyricsBySongId_parse(char* data, opensubsonic_getLyricsBySongId_struct** getLyricsBySongIdStruct);
void opensubsonic_getLyricsBySongId_struct_free(opensubsonic_getLyricsBySongId_struct** getLyricsBySongIdStruct);

#endif

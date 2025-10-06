#ifndef _ENDPOINT_GETSONG_H
#define _ENDPOINT_GETSONG_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    char* status;       // Request status
    int errorCode;      // Request error code (0 if none)
    char* errorMessage; // Request error message (NULL if none)
    char* id;           // Song ID
    char* parent;       // Song parent
    // 'isDir' excluded
    char* title;        // Song title
    char* album;        // Song album
    char* artist;       // Song artist
    int track;          // Track number
    int year;           // Year released
    char* coverArt;     // Cover art ID
    long size;          // Size of the song in bytes
    // 'contentType', 'suffix' excluded
    char* starred;      // Time starred
    long duration;      // Duration of the song in seconds
    int bitRate;        // Bitrate of the song
    int bitDepth;       // Bit depth
    long samplingRate;  // Sample rate
    int channelCount;   // Channel count
    char* played;       // Time last played (TODO probably)
    int discNumber;     // Disc number
    char* created;      // Date added (TODO probably)
    char* albumId;      // Album ID
    char* artistId;     // Artist ID
    // 'type', 'mediaType', 'isVideo', 'bpm', 'comment', 'sortName', 'musicBrainzId', 'genres', 'artists', 'albumArtists', 'contributors', 'moods', 'replayGain' excluded
    char* displayArtist;        // Display Artist
    char* displayAlbumArtist;   // Display Album Artist
    char* displayComposer;      // Display Composer
} opensubsonic_getSong_struct;

int opensubsonic_getSong_parse(char* data, opensubsonic_getSong_struct** getSongStruct);
void opensubsonic_getSong_struct_free(opensubsonic_getSong_struct** getSongStruct);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _ENDPOINT_GETSONG_H

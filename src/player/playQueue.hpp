/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 */

#ifndef _PLAYQUEUE_H
#define _PLAYQUEUE_H

#define OSSPQ_MODE_OPENSUBSONIC 101
#define OSSPQ_MODE_LOCALFILE 102
#define OSSPQ_MODE_INTERNETRADIO 103

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// C interface for sending song queue data (C++ interface is in the C++ file)
typedef struct {
    char* title;
    char* album;
    char* artist;
    char* id;
    char* streamUrl;
    char* coverArtUrl;
    long duration;
    int mode;
} OSSPQ_SongStruct;

int OSSPQ_AppendToEnd(char* title, char* album, char* artist, char* id, char* streamUrl, char* coverArtUrl, long duration, int mode);
OSSPQ_SongStruct* OSSPQ_PopFromFront();
void OSSPQ_FreeSongObjectC(OSSPQ_SongStruct* songObjectC);

// TODO
char* internal_OSSPQ_GetTitleAtIndex(int idx);
int internal_OSSPQ_GetItemCount();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PLAYQUEUE_H

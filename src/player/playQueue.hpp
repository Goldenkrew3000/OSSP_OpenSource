/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 */

#ifndef _PLAYQUEUE_H
#define _PLAYQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// C interface for sending song queue data (C++ interface is in the C++ file)
typedef struct {
    char* title;
    char* artist;
    char* id;
    long duration;
} OSSPQ_SongStruct;

int internal_OSSPQ_AppendToEnd(char* title, char* artist, char* id, long duration);
OSSPQ_SongStruct* internal_OSSPQ_PopFromFront();
void internal_OSSPQ_FreeSongObject(OSSPQ_SongStruct* songObject);
char* internal_OSSPQ_GetTitleAtIndex(int idx);
int internal_OSSPQ_GetItemCount();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PLAYQUEUE_H

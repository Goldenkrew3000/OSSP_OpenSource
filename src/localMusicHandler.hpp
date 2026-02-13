/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 */

#ifndef _LOCALMUSICHANDLER_H
#define _LOCALMUSICHANDLER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void localMusicHandler_scan();
void localMusicHandler_scanDirectory(char* directory);
void localMusicHandler_scanFile(int idx);
void localMusicHandler_generateUid(int idx);

void localMusicHandler_initDatabase();
void localMusicHandler_moveSongsToDatabase(int idx);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _LOCALMUSICHANDLER_H

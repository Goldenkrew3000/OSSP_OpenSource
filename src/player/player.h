/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 */

#ifndef _PLAYER_H
#define _PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "playQueue.hpp"

void* OSSPlayer_GMainLoop(void* arg);
void* OSSPlayer_ThrdInit(void* arg);
int OSSPlayer_GstInit();
int OSSPlayer_QueueAppend_Song(char* title, char* artist, char* id, long duration);
int OSSPlayer_QueueAppend_Radio(char* name, char* id, char* radioUrl);
OSSPQ_SongStruct* OSSPlayer_QueuePopFront();

float OSSPlayer_GstECont_InVolume_Get();
void OSSPlayer_GstECont_InVolume_set(float val);
float OSSPlayer_GstECont_OutVolume_Get();
void OSSPlayer_GstECont_OutVolume_set(float val);
float OSSPlayer_GstECont_Pitch_Get();
void OSSPlayer_GstECont_Pitch_Set(float cents);
void OSSPlayer_GstECont_Playbin3_Stop();
void OSSPlayer_GstECont_Playbin3_PlayPause();

float OSSPlayer_DbLinMul(float db);
float OSSPlayer_PitchFollow(float freq, float semitone);
float OSSPlayer_CentsToPSF(float cents);

#ifdef __cplusplus
}
#endif

#endif

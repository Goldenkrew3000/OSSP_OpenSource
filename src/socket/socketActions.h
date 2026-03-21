/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Socket Actions
 */

#ifndef _SOCKETACTIONS_H
#define _SOCKETACTIONS_H
#include "../external/cJSON.h"

#define OSSP_SOCKET_ACTION_GET_STARRED_SONGS 101
#define OSSP_SOCKET_ACTION_NOW_PLAYING 201
#define OSSP_SOCKET_ACTION_STATS 202
#define OSSP_SOCKET_ACTION_ADD_TO_QUEUE 203

#define OSSP_SOCKET_ACTION_OSSPP_PREV 301
#define OSSP_SOCKET_ACTION_OSSPP_PLAYPAUSE 302
#define OSSP_SOCKET_ACTION_OSSPP_NEXT 303
#define OSSP_SOCKET_ACTION_OSSPP_OUTVOLUME 304
#define OSSP_SOCKET_ACTION_OSSPP_INVOLUME 305

void OSSPS_SocketAction_Get_Starred_Songs(char** retDataStr, cJSON** cliReqJson);
void OSSPS_SocketAction_Now_Playing(char** retDataStr, cJSON** cliReqJson);
void OSSPS_SocketAction_Add_To_Queue(char** retDataStr, cJSON** cliReqJson);
void OSSPS_SocketAction_OSSPP_OutVolume(char** retDataStr, cJSON** cliReqJson);
void OSSPS_SocketAction_OSSPP_InVolume(char** retDataStr, cJSON** cliReqJson);

#endif

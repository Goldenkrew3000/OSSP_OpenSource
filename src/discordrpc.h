/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 */

#ifndef _DISCORDRPC_H
#define _DISCORDRPC_H

#define DISCORDRPC_STATE_IDLE 0
#define DISCORDRPC_STATE_PLAYING 1
#define DISCORDRPC_STATE_PAUSED 2

typedef struct {
    int state;
    long songLength;
    char* songTitle;
    char* songArtist;
    char* coverArtUrl;
} discordrpc_data;


void discordrpc_struct_init(discordrpc_data** discordrpc_struct);
void discordrpc_struct_deinit(discordrpc_data** discordrpc_struct);
void discordrpc_init();
void discordrpc_update(discordrpc_data** discordrpc_struct);
char* discordrpc_getOS();

#endif

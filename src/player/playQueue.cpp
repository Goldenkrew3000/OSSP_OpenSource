/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Gstreamer Queue Handler
 */

 /*
  * Now you might ask why this even exists in this way, but I thought that it would be easier to jump to C++
  * to store a song queue with objects instead of some hacked together solution in C
  * And I was right.
  * Like yeah, you can store an array of structs easily in C, but to dynamically be able to move those around, no,
  * std::deque makes that MUCH simpler.
  */

#include <cstddef>
#include <string.h>
#include <string>
#include <deque>
#include "playQueue.hpp"

// C++ interface for storing song queue data (C interface is in the header)
class OSSPQ_SongObject {
    public:
        std::string title;
        std::string album;
        std::string artist;
        std::string id;
        std::string streamUrl;
        std::string coverArtUrl;
        long duration;
        int mode;
};

// NOTE: Acronym is OpenSubsonicPlayerQueue
std::deque<OSSPQ_SongObject> OSSPQ_SongQueue;

int OSSPQ_AppendToEnd(char* title, char* album, char* artist, char* id, char* streamUrl, char* coverArtUrl, long duration, int mode) {
    OSSPQ_SongObject songObject;

    if (mode == OSSPQ_MODE_OPENSUBSONIC || mode == OSSPQ_MODE_LOCALFILE) {
        // Both Local File and OpenSubsonic playback both take the same arguments
        if (mode == OSSPQ_MODE_OPENSUBSONIC) {
            printf("[OSSPQ] Appending OpenSubsonic Song: %s by %s.\n", title, artist);
        } else if (mode == OSSPQ_MODE_LOCALFILE) {
            printf("[OSSPQ] Appending Local Song: %s by %s.\n", title, artist);
        }

        songObject.title = title;
        songObject.album = album;
        songObject.artist = artist;
        songObject.id = id;
        songObject.streamUrl = streamUrl;
        songObject.coverArtUrl = coverArtUrl;
        songObject.duration = duration;
        songObject.mode = mode;
    } else if (mode == OSSPQ_MODE_INTERNETRADIO) {
        printf("[OSSPQ] Appending Internet Radio Station: %s.\n", title);
        
        songObject.title = title;
        songObject.id = id;
        songObject.streamUrl = streamUrl;
    }

    OSSPQ_SongQueue.push_back(songObject);
    return 0;
}

OSSPQ_SongStruct* OSSPQ_PopFromFront() {
    // Check if song queue is empty, if not, then pop oldest
    if (OSSPQ_SongQueue.empty()) {
        return NULL;
    }
    OSSPQ_SongObject songObject = OSSPQ_SongQueue.front();
    OSSPQ_SongQueue.pop_front();

    // Allocate, initialize, and fill C compatible song object
    OSSPQ_SongStruct* songObjectC = (OSSPQ_SongStruct*)malloc(sizeof(OSSPQ_SongStruct));
    songObjectC->title = NULL;
    songObjectC->album = NULL;
    songObjectC->artist = NULL;
    songObjectC->id = NULL;
    songObjectC->streamUrl = NULL;
    songObjectC->coverArtUrl = NULL;
    songObjectC->duration = 0;
    songObjectC->mode = 0;

    if (songObject.mode == OSSPQ_MODE_OPENSUBSONIC || songObject.mode == OSSPQ_MODE_LOCALFILE) {
        songObjectC->title = strdup(songObject.title.c_str());
        songObjectC->album = strdup(songObject.album.c_str());
        songObjectC->artist = strdup(songObject.artist.c_str());
        songObjectC->id = strdup(songObject.id.c_str());
        songObjectC->streamUrl = strdup(songObject.streamUrl.c_str());
        songObjectC->coverArtUrl = strdup(songObject.coverArtUrl.c_str());
        songObjectC->duration = songObject.duration;
        songObjectC->mode = songObject.mode;
    } else if (songObject.mode == OSSPQ_MODE_INTERNETRADIO) {
        songObjectC->title = strdup(songObject.title.c_str());
        songObjectC->id = strdup(songObject.id.c_str());
        songObjectC->streamUrl = strdup(songObject.streamUrl.c_str());
    }
    
    return songObjectC;
}

void OSSPQ_FreeSongObjectC(OSSPQ_SongStruct* songObjectC) {
    printf("[OSSPQ] Freeing SongObjectC.\n");
    if (songObjectC->title != NULL) { free(songObjectC->title); }
    if (songObjectC->album != NULL) { free(songObjectC->album); }
    if (songObjectC->artist != NULL) { free(songObjectC->artist); }
    if (songObjectC->id != NULL) { free(songObjectC->id); }
    if (songObjectC->streamUrl != NULL) { free(songObjectC->streamUrl); }
    if (songObjectC->coverArtUrl != NULL) { free(songObjectC->coverArtUrl); }
    if (songObjectC != NULL) { free(songObjectC); }
}

// TODO Bullshit functions for dealing with Imgui
char* internal_OSSPQ_GetTitleAtIndex(int idx) {
    return (char*)OSSPQ_SongQueue[idx].title.c_str();
}

int internal_OSSPQ_GetItemCount() {
    return OSSPQ_SongQueue.size();
}

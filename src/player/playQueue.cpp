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
class SongObject {
    public:
        std::string title;
        std::string artist;
        std::string id;
        long duration;
};

// NOTE: Acronym is OpenSubsonicPlayerQueue
std::deque<SongObject> OSSPQ_Items;

int internal_OSSPQ_AppendToEnd(char* title, char* artist, char* id, long duration) {
    // Append a new song to the end of the queue
    printf("Title: %s\nArtist: %s\nID: %s\nDuration: %ld\n", title, artist, id, duration);
    // TODO: Find a neater way of converting a C string to a C++ string??
    std::string cpp_title(title);
    std::string cpp_artist(artist);
    std::string cpp_id(id);
    SongObject songObject;
    songObject.title = cpp_title;
    songObject.artist = cpp_artist;
    songObject.id = cpp_id;
    songObject.duration = duration;
    OSSPQ_Items.push_back(songObject);
    return 0;
}

OSSPQ_SongStruct* internal_OSSPQ_PopFromFront() {
    if (OSSPQ_Items.empty()) {
        // No items in play queue
        return NULL;
    }

    // Pull the first song off the song queue
    SongObject songObject = OSSPQ_Items.front();
    OSSPQ_Items.pop_front();

    // Move song data into a C readable format
    // NOTE: I am initializing the variables to a known value just in case there is missing information in songObject
    OSSPQ_SongStruct* playQueueObject = (OSSPQ_SongStruct*)malloc(sizeof(OSSPQ_SongStruct));
    playQueueObject->title = NULL;
    playQueueObject->artist = NULL;
    playQueueObject->id = NULL;
    playQueueObject->duration = 0;

    playQueueObject->title = strdup(songObject.title.c_str());
    playQueueObject->artist = strdup(songObject.artist.c_str());
    playQueueObject->id = strdup(songObject.id.c_str());
    playQueueObject->duration = songObject.duration;
    return playQueueObject;
}

void internal_OSSPQ_FreeSongObject(OSSPQ_SongStruct* songObject) {
    if (songObject->title != NULL) { free(songObject->title); }
    if (songObject->artist != NULL) { free(songObject->artist); }
    if (songObject->id != NULL) { free(songObject->id); }
    if (songObject != NULL) { free(songObject); }
}

char* internal_OSSPQ_GetTitleAtIndex(int idx) {
    return (char*)OSSPQ_Items[idx].title.c_str();
}

int internal_OSSPQ_GetItemCount() {
    return OSSPQ_Items.size();
}

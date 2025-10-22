/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Gstreamer Queue Handler
 */

#include <string.h>
#include <string>
#include <deque>
#include "playQueue.hpp"

// NOTE: Acronym is OpenSubsonicPlayerQueue
std::deque<std::string> OSSPQ_Items;

int internal_OSSPQ_AppendToEnd(char* id) {
    std::string cpp_id(id);
    OSSPQ_Items.push_back(cpp_id);
    return 0;
}

char* internal_OSSPQ_PopFromFront() {
    if (OSSPQ_Items.empty()) {
        // No items in play queue
        return NULL;
    }

    char* id = strdup(OSSPQ_Items.front().c_str()); // Heap allocate id
    OSSPQ_Items.pop_front();
    return id;
}

int internal_OSSPQ_GetItemCount() {
    return OSSPQ_Items.size();
}

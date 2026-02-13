/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Local Music File Handler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include "localMusicHandler.hpp"
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/dict.h>
    #include <libavformat/avio.h>
}

void localMusicHandler_scan() {
    //
}

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
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/dict.h>
    #include <libavformat/avio.h>
    #include "external/sqlite3/sqlite3.h"
}
#include <iostream>
#include <regex>
#include <vector>
#include <deque>
#include "configHandler.h"
#include "localMusicHandler.hpp"

/*
 * I'm sorry for this messy and probably unreliable code
 * This is the first time I have ever written something like this
 * And why C++? Easy to store all of the temporary data without a ton of boilerplate
 */

extern configHandler_config_t* configObj;
std::vector<std::string> localMusicHandler_allFiles;
class localMusicHandler_AudioObject {
    public:
        std::string path;
        std::string songTitle;
        std::string albumTitle;
        std::string artistTitle;
        std::string track;
        std::string totalTracks;
        uint64_t filesize;
        std::string uid;
};
std::deque<localMusicHandler_AudioObject> localMusicHandler_audioItems;
static sqlite3* sqlite_db = NULL;
static char* sqlite_errorMsg = NULL;

void localMusicHandler_scan() {
    static int rc = 0;
    printf("[LocalMusicHandler] Scanning local music directory recursively for files.\n");

    // TODO clear all vectors

    // Scan music directory (defined in config file) for all files
    localMusicHandler_scanDirectory(configObj->local_rootdir);
    printf("[LocalMusicHandler] Found %d files.\n", localMusicHandler_allFiles.size());

    // Scan each file to find only music files, and pull ID3 tags
    for (int i = 0; i < localMusicHandler_allFiles.size(); i++) {
        localMusicHandler_scanFile(i);
    }
    printf("[LocalMusicHandler] Found %d songs.\n", localMusicHandler_audioItems.size());

    // Generate a unique ID for each music file
    printf("[LocalMusicHandler] Generating unique IDs for each song.\n");
    for (int i = 0; i < localMusicHandler_audioItems.size(); i++) {
        // TODO: Technically there is a VERY SMALL chance that 2 id's repeat in the DB
        //       Figure out what to do about that later
        localMusicHandler_generateUid(i);
    }

    // Store in database
    rc = localMusicHandler_initDatabase();
    if (rc == -1) {
        // ERROR
    } else if (rc == 0) {
        // Table just made, songs not loaded in yet
        for (int i = 0; i < localMusicHandler_audioItems.size(); i++) {
            localMusicHandler_moveSongsToDatabase(i);
        }
    } else if (rc == 1) {
        // Table was already made, assume songs were loaded in before
    }
}

void localMusicHandler_scanDirectory(char* directory) {
    struct dirent* dp;
    DIR* dir = opendir(directory);
    char path[1000]; // TODO Prevent potential buffer overflow

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            sprintf(path, "%s/%s", directory, dp->d_name);

            struct stat statbuf;
            stat(path, &statbuf);

            if (S_ISDIR(statbuf.st_mode)) {
                localMusicHandler_scanDirectory(path);
            } else if (S_ISREG(statbuf.st_mode)) {
                localMusicHandler_allFiles.push_back(path);
            }
        }
    }

    closedir(dir);
}

void localMusicHandler_scanFile(int idx) {
    AVFormatContext* ctx = NULL;
    AVDictionaryEntry* tag = NULL;
    static int rc = -1;

    rc = avformat_open_input(&ctx, localMusicHandler_allFiles[idx].c_str(), NULL, NULL);
    if (rc < 0) {
        printf("[LocalMusicHandler] avformat_open_input() failed on idx %d (%s).\n", idx, localMusicHandler_allFiles[idx].c_str());
        return;
    }

    // Ignore files that aren't audio
    if (
        strcmp(ctx->iformat->name, "lrc") == 0 ||                   // .lrc files
        strcmp(ctx->iformat->name, "image2") == 0 ||                // Pictures
        strcmp(ctx->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2") == 0  // .mp4 files
    ) {
        avformat_close_input(&ctx);
        return;
    }

    localMusicHandler_AudioObject audioObject;
    audioObject.path = localMusicHandler_allFiles[idx].c_str();

    // Get file size (Using libav for this since the file is already opened using it)
    audioObject.filesize = 0; // If the following fetch fails, set it to a known value beforehand
    if (ctx->pb) {
        uint64_t fsize = avio_size(ctx->pb);
        if (fsize > 0) {
            audioObject.filesize = fsize;
        }
    }

    // Set all strings to known good values before fetching tags that possible don't exist
    // NOTE: Honestly don't know if C++ does this by default, but I am not trusting it either way
    audioObject.songTitle = "";
    audioObject.albumTitle = "";
    audioObject.artistTitle = "";
    audioObject.track = "";
    audioObject.totalTracks = "";

    while ((tag = av_dict_get(ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        if (strcmp(tag->key, "title") == 0) {
            audioObject.songTitle = tag->value;
        } else if (strcmp(tag->key, "album") == 0) {
            audioObject.albumTitle = tag->value;
        } else if (strcmp(tag->key, "artist") == 0) {
            // In ID3, multiple artists are stored as 'Artist A;Artist B'. Replace ';' with ', '
            audioObject.artistTitle = std::regex_replace(tag->value, std::regex(";"), ", ");
        } else if (strcmp(tag->key, "track") == 0) {
            audioObject.track = tag->value;
        } else if (strcmp(tag->key, "totaltracks") == 0) {
            audioObject.totalTracks = tag->value;
        }
    }

    localMusicHandler_audioItems.push_back(audioObject);
    avformat_close_input(&ctx);
}

void localMusicHandler_generateUid(int idx) {
    // TODO: Add other operating support here, such as in libopensubsonic/crypto.c
    char uuidBytes[20];
    char uuidString[40];
    for (int i = 0; i < 20; i++) {
        uuidBytes[i] = arc4random() & 0xFF;
    }
    snprintf(uuidString, 40, "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",
        uuidBytes[0], uuidBytes[1], uuidBytes[2], uuidBytes[3], uuidBytes[4],
        uuidBytes[5], uuidBytes[6], uuidBytes[7], uuidBytes[8], uuidBytes[9],
        uuidBytes[10], uuidBytes[11], uuidBytes[12], uuidBytes[13], uuidBytes[14],
        uuidBytes[15], uuidBytes[16], uuidBytes[17], uuidBytes[18], uuidBytes[19]
    );
    localMusicHandler_audioItems[idx].uid = uuidString;
}

int localMusicHandler_initDatabase() {
    // Code returns: -1 -> Error, 0 -> No songs in table, 1 -> Songs already in table (Table already existed)
    static int createTable = 0;
    static int rc = 0;
    char* dbPath = NULL;
    rc = asprintf(&dbPath, "%s/.config/ossp/local.db", getenv("HOME"));
    if (rc == -1) {
        printf("[LocalMusicHandler] asprintf() failed.\n");
        return -1;
    }

    struct stat st;
    if (stat(dbPath, &st) == 0) {
        printf("[LocalMusicHandler] Database found, is %ld bytes.\n", st.st_size);
    } else { 
        printf("[LocalMusicHandler] Database does not exist, creating.\n");
        createTable = 1;
    }

    rc = sqlite3_open(dbPath, &sqlite_db);
    if (rc) {
        printf("[LocalMusicHandler] Could not create database: %s\n", sqlite3_errmsg(sqlite_db));
        free(dbPath);
        return -1;
    } else {
        printf("[LocalMusicHandler] Created/Opened database.\n");
    }

    if (createTable == 1) {
        const char* sqlQuery = "CREATE TABLE local_songs(uid TEXT, songTitle TEXT, albumTitle TEXT, artistTitle TEXT, track TEXT, totalTracks TEXT, path TEXT, filesize INT)";
        rc = sqlite3_exec(sqlite_db, sqlQuery, NULL, 0, &sqlite_errorMsg);
        if (rc != SQLITE_OK) {
            printf("[LocalMusicHandler] Could not make table: %s\n", sqlite_errorMsg);
            sqlite3_free(sqlite_errorMsg);
            free(dbPath);
            return -1;
        }
        printf("[LocalMusicHandler] Made table.\n");
        free(dbPath);
        return 0;
    }

    free(dbPath);
    return 1;
}

void localMusicHandler_moveSongsToDatabase(int idx) {
    sqlite3_stmt* sqlite_stmt;
    const char* sqlQuery = "INSERT INTO local_songs VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

    if (sqlite3_prepare_v2(sqlite_db, sqlQuery, -1, &sqlite_stmt, NULL) != SQLITE_OK) {
        printf("[LocalMusicHandler] Prepare error: %s\n", sqlite3_errmsg(sqlite_db));
        return; // TODO
    }

    sqlite3_bind_text(sqlite_stmt, 1, localMusicHandler_audioItems[idx].uid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(sqlite_stmt, 2, localMusicHandler_audioItems[idx].songTitle.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(sqlite_stmt, 3, localMusicHandler_audioItems[idx].albumTitle.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(sqlite_stmt, 4, localMusicHandler_audioItems[idx].artistTitle.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(sqlite_stmt, 5, localMusicHandler_audioItems[idx].track.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(sqlite_stmt, 6, localMusicHandler_audioItems[idx].totalTracks.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(sqlite_stmt, 7, localMusicHandler_audioItems[idx].path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(sqlite_stmt, 8, localMusicHandler_audioItems[idx].filesize);

    if (sqlite3_step(sqlite_stmt) != SQLITE_DONE) {
        printf("[LocalMusicHandler] Execution error: %s\n", sqlite3_errmsg(sqlite_db));
    }

    sqlite3_finalize(sqlite_stmt);
}

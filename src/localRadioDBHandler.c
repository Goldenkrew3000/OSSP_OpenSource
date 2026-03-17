/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2026
 * License: GNU General Public License 3.0
 * Info: Local (Not from OpenSubsonic server) Internet Radio Sqlite3 Database Handler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "external/sqlite3/sqlite3.h"

#include "localRadioDBHandler.h"

static sqlite3* db = NULL;
static int rc = 0;
static char* errorMsg = NULL;

int localRadioDBHandler_CreateDB(char* dbPath);

int localRadioDBHandler_Init() {
    // Form path
    char* homePath = getenv("HOME");
    char* dbPath = NULL;
    rc = asprintf(&dbPath, "%s/.config/ossp/radio.db", homePath);
    if (rc == -1) { printf("asprintf() failed.\n"); return 1; }

    // Check if file exists
    struct stat st;
    if (stat(dbPath, &st) == 0) {
        printf("[LocalRadio] Database found, is %ld bytes.\n", st.st_size);
    } else { 
        printf("[LocalRadio] Database does not exist, creating.\n");
        localRadioDBHandler_CreateDB(dbPath);
    }
}

int localRadioDBHandler_CreateDB(char* dbPath) {
    // Create database
    rc = sqlite3_open(dbPath, &db);
    if (rc) {
        printf("Could not create database: %s\n", sqlite3_errmsg(db));
    } else { printf("Created database.\n"); }

    // Create table 'stations'
    char* sqlQuery = "CREATE TABLE stations(id INT, name TEXT, url TEXT)";
    rc = sqlite3_exec(db, sqlQuery, NULL, 0, &errorMsg);
    if (rc != SQLITE_OK) {
        printf("could not make table: %s\n", errorMsg);
        sqlite3_free(errorMsg);
    } else {
        printf("made table\n");
    }

    sqlite3_close(db);
}

int localRadioDBHandler_AddStation(char* name, char* url) {
    //
}

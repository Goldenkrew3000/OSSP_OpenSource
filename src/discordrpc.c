/*
 * OpenSubSonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Discord Local RPC Handler
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "external/discord-rpc/include/discord_rpc.h"
#include "libopensubsonic/logger.h"
#include "configHandler.h"
#include "discordrpc.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/sysctl.h>
#endif // defined(__APPLE__) && defined(__MACH__)

extern configHandler_config_t* configObj;
const char* discordrpc_appid = "1407025303779278980";
char* discordrpc_osString = NULL;
static int rc = 0;

void discordrpc_struct_init(discordrpc_data** discordrpc_struct) {
    (*discordrpc_struct) = malloc(sizeof(discordrpc_data));
    (*discordrpc_struct)->state = 0;
    (*discordrpc_struct)->songLength = 0;
    (*discordrpc_struct)->songTitle = NULL;
    (*discordrpc_struct)->songArtist = NULL;
    (*discordrpc_struct)->coverArtUrl = NULL;
}

void discordrpc_struct_deinit(discordrpc_data** discordrpc_struct) {
    if ((*discordrpc_struct)->songTitle != NULL) { free((*discordrpc_struct)->songTitle); }
    if ((*discordrpc_struct)->songArtist != NULL) { free((*discordrpc_struct)->songArtist); }
    if ((*discordrpc_struct)->coverArtUrl != NULL) { free((*discordrpc_struct)->coverArtUrl); }
    if (*discordrpc_struct != NULL) { free(*discordrpc_struct); }
}

static void handleDiscordReady(const DiscordUser* connectedUser)
{
    printf("\nDiscord: connected to user %s#%s - %s\n",
           connectedUser->username,
           connectedUser->discriminator,
           connectedUser->userId);
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
    printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

static void handleDiscordError(int errcode, const char* message)
{
    printf("\nDiscord: error (%d: %s)\n", errcode, message);
}

int discordrpc_init() {
    printf("[DiscordRPC] Initializing.\n");
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    Discord_Initialize(discordrpc_appid, &handlers, 1, NULL);

    // Fetch OS String for RPC (Heap-allocated)
    discordrpc_osString = discordrpc_getOS();
    if (discordrpc_osString == NULL) {
        logger_log_error(__func__, "asprintf() or strdup() failed.");
        return 1;
    }
    return 0;
}

void discordrpc_update(discordrpc_data** discordrpc_struct) {
    printf("[DiscordRPC] Updating...\n");
    DiscordRichPresence presence;
    char* detailsString = NULL;
    char* stateString = NULL;
    memset(&presence, 0, sizeof(presence));

    if ((*discordrpc_struct)->state == DISCORDRPC_STATE_IDLE) {
        asprintf(&detailsString, "Idle");
        presence.details = detailsString;
    } else if ((*discordrpc_struct)->state == DISCORDRPC_STATE_PLAYING_SONG) {
        // Playing a song
        time_t currentTime = time(NULL);
        asprintf(&detailsString, "%s", (*discordrpc_struct)->songTitle);
        asprintf(&stateString, "by %s", (*discordrpc_struct)->songArtist);
        presence.details = detailsString;
        presence.state = stateString;
        presence.largeImageKey = (*discordrpc_struct)->coverArtUrl;
        presence.startTimestamp = (long)currentTime;
        presence.endTimestamp = (long)currentTime + (*discordrpc_struct)->songLength;
        if (configObj->discordrpc_showSysDetails) {
            presence.largeImageText = discordrpc_osString;
        }
    } else if ((*discordrpc_struct)->state == DISCORDRPC_STATE_PLAYING_RADIO) {
        // Playing an internet radio station
        time_t currentTime = time(NULL);
        asprintf(&detailsString, "%s", (*discordrpc_struct)->songTitle);
        asprintf(&stateString, "Internet radio station");
        presence.details = detailsString;
        presence.state = stateString;
        presence.largeImageKey = (*discordrpc_struct)->coverArtUrl;
        presence.startTimestamp = (long)currentTime;
        if (configObj->discordrpc_showSysDetails) {
            presence.largeImageText = discordrpc_osString;
        }
    } else if ((*discordrpc_struct)->state == DISCORDRPC_STATE_PAUSED) {

    }

    presence.activity_type = DISCORD_ACTIVITY_TYPE_LISTENING;
    Discord_UpdatePresence(&presence);

    free(detailsString);
    if (stateString != NULL) { free(stateString); }
}

char* discordrpc_getOS() {
#if defined(__linux__)
    // NOTE: Could have made a sysctl function, but this is literally only done here, not worth it
    FILE* fp_ostype = fopen("/proc/sys/kernel/ostype", "r");
    char buf_ostype[16];
    if (!fp_ostype) {
        logger_log_error(__func__, "Could not perform kernel.ostype sysctl.");
        return NULL;
    }

    FILE* fp_osrelease = fopen("/proc/sys/kernel/osrelease", "r");
    char buf_osrelease[32];
    if (!fp_osrelease) {
        logger_log_error(__func__, "Could not perform kernel.osrelease sysctl.");
        return NULL;
    }

    FILE* fp_osarch = fopen("/proc/sys/kernel/arch", "r");
    char buf_osarch[16];
    if (!fp_osarch) {
        logger_log_error(__func__, "Could not perform kernel.arch sysctl.");
        return NULL;
    }

    if (fgets(buf_ostype, sizeof(buf_ostype), fp_ostype) == NULL) {
        logger_log_error(__func__, "Could not perform kernel.ostype sysctl.");
        fclose(fp_ostype);
        fclose(fp_osrelease);
        fclose(fp_osarch);
        return NULL;
    }
    if (fgets(buf_osrelease, sizeof(buf_osrelease), fp_osrelease) == NULL) {
        logger_log_error(__func__, "Could not perform kernel.osrelease sysctl.");
        fclose(fp_ostype);
        fclose(fp_osrelease);
        fclose(fp_osarch);
        return NULL;
    }
    if (fgets(buf_osarch, sizeof(buf_osarch), fp_osarch) == NULL) {
        logger_log_error(__func__, "Could not perform kernel.arch sysctl.");
        fclose(fp_ostype);
        fclose(fp_osrelease);
        fclose(fp_osarch);
        return NULL;
    }
    fclose(fp_ostype);
    fclose(fp_osrelease);
    fclose(fp_osarch);

    // HACK: Since Linux removed the sysctl interface, I have to manually remove newlines from the /proc contents
    buf_ostype[strcspn(buf_ostype, "\n")] = '\0';
    buf_osrelease[strcspn(buf_osrelease, "\n")] = '\0';
    buf_osarch[strcspn(buf_osarch, "\n")] = '\0';

    char* osString = NULL;
    rc = asprintf(&osString, "on %s %s %s", buf_ostype, buf_osarch, buf_osrelease);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return NULL;
    }
    return osString;
#elif defined(__APPLE__) && defined(__MACH__)
    // NOTE: Okay so I _could_ just print 'Darwin' for the OS Type, but on the 0.0001% chance that this is running on
    //       OpenDarwin / PureDarwin, I am fetching the name using a sysctl
    char buf_ostype[16];
    size_t sz_ostype = sizeof(buf_ostype);
    char buf_osrelease[16];
    size_t sz_osrelease = sizeof(buf_osrelease);
    int isArm64 = 0;
    size_t sz_isArm64 = sizeof(isArm64);
    int mib[CTL_MAXNAME];

    mib[0] = CTL_KERN;
    mib[1] = KERN_OSTYPE;
    if (sysctl(mib, 2, buf_ostype, &sz_ostype, NULL, 0) == -1) {
        logger_log_error(__func__, "Could not perform kern.ostype sysctl.");
        return NULL;
    }

    mib[1] = KERN_OSRELEASE;
    if (sysctl(mib, 2, buf_osrelease, &sz_osrelease, NULL, 0) == -1) {
        logger_log_error(__func__, "Could not perform kern.osrelease sysctl.");
        return NULL;
    }

    // hw.optional.arm64 does not seem to have a direct mib0/1 route
    size_t mib_len = CTL_MAXNAME;
    if (sysctlnametomib("hw.optional.arm64", mib, &mib_len) != 0) {
        logger_log_error(__func__, "Could not perform hw.optional.arm64 sysctl.");
        return NULL;
    }
    if (sysctl(mib, mib_len, &isArm64, &sz_isArm64, NULL, 0) != 0) {
        logger_log_error(__func__, "Could not perform hw.optional.arm64 sysctl.");
        return NULL;
    }

    char* osString = NULL;
    if (isArm64 == 1) {
        rc = asprintf(&osString, "on %s XNU aarch64 %s", buf_ostype, buf_osrelease);
    } else {
        rc = asprintf(&osString, "on %s XNU x86_64 %s", buf_ostype, buf_osrelease);
    }
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return NULL;
    }
    return osString;
#else
    // NOTE: This is not a critical error, just let the user know
    logger_log_error(__func__, "Could not fetch OS details.");
    return strdup("on Unknown");
#endif
}

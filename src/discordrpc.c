/*
 * OpenSubSonicPlayer
 * Goldenkrew3000 2025
 * GPL-3.0
 * Discord Local RPC Handler
 * Note: This provides server auth creds (encoded) directly to Discord, could use Spotify's API instead??
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "external/discord-rpc/include/discord_rpc.h"
#include "discordrpc.h"

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

void discordrpc_init() {
    printf("[DiscordRPC] Initializing...\n");
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    Discord_Initialize(discordrpc_appid, &handlers, 1, NULL);

    // Fetch OS String for RPC (Heap-allocated)
    // TODO: Check if failed
    discordrpc_osString = discordrpc_getOS();
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
    } else if ((*discordrpc_struct)->state == DISCORDRPC_STATE_PLAYING) {
        asprintf(&detailsString, "%s", (*discordrpc_struct)->songTitle);
        asprintf(&stateString, "by %s", (*discordrpc_struct)->songArtist);
        presence.details = detailsString;
        presence.state = stateString;
        // TODO: Discord is currently broken for this rn
        //presence.largeImageKey = (*discordrpc_struct)->coverArtUrl;
        presence.largeImageText = discordrpc_osString;
    } else if ((*discordrpc_struct)->state == DISCORDRPC_STATE_PAUSED) {

    }

    presence.activity_type = DISCORD_ACTIVITY_TYPE_LISTENING;
    Discord_UpdatePresence(&presence);

    free(detailsString);
    if (stateString != NULL) { free(stateString); }
}

char* discordrpc_getOS() {
    // NOTE: Could have made a sysctl function, but this is literally only done here, not worth it
    // TODO: This is ONLY linux compatible at this point

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
}

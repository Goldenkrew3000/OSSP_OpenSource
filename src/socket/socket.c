/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Socket Handler
 */

/*
 * Socket Policy:
 * - The client shall not send a URL back to the server
 * - The client shall only send back IDs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../external/cJSON.h"
#include "../configHandler.h"
#include "../player/player.h"
#include "../player/playQueue.hpp"
#include "../libopensubsonic/utils.h"
#include "socket.h"
#include "socketActions.h"

static int server_fd = -1;
static int client_fd = -1;
static int rc = -1;
socklen_t client_len;
struct sockaddr_un server_addr;
struct sockaddr_un client_addr;
extern configHandler_config_t* configObj;

bool isSocketHandlerLoopRunning = false;



void socketHandler_read();
void socketHandler_performAction(int id, char** retDataStr, cJSON** cliReqJson);

int SockSig_Length = 4;

const uint32_t OSSP_Sock_ACK = 0x7253FF87;
const uint32_t OSSP_Sock_CliConn = 0xE3566C2E;
const uint32_t OSSP_Sock_GetConnInfo = 0x8E4F6B01;
const uint32_t OSSP_Sock_Size = 0x1F7E8BCF;
const uint32_t OSSP_Sock_ClientGetReq = 0x210829CF;
const uint32_t OSSP_Sock_FragMsgACK = 0x32A093B4;



int socketHandler_init() {
    printf("[SocketHandler] Initializing.\n");

    // Create server socket, and ensure that the socket file is removed
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("[SocketHandler] Could not open server socket.\n");
        // TODO
    }

    unlink(configObj->client_socket_path);

    // Bind server socket to SOCKET_PATH
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, configObj->client_socket_path, sizeof(server_addr.sun_path) - 1);

    rc = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_un));
    if (rc == -1) {
        printf("[SocketHandler] Could not bind server socket.\n");
        // TODO
    }

    rc = listen(server_fd, 5);
    if (rc == -1) {
        printf("[SocketHandler] Could not listen on server socket.\n");
        // TODO
    }

    // Wait for connection
    bool isServerWaiting = true;
    while (isServerWaiting) {
        client_fd = accept(server_fd, NULL, NULL); // TODO
        if (client_fd == -1) {
            printf("[SocketHandler] Error accepting connection.\n");
        } else {
            printf("[SocketHandler] Accepted connection.\n");
            isServerWaiting = false;
        }
    }

    // Initialize connection with client
    socketHandler_initClientConnection();

    // Enter socket handler loop
    isSocketHandlerLoopRunning = true;
    //while (isSocketHandlerLoopRunning) {
        //
    //}



    printf("------------------------------\n");

    while (1) {
        // Receive ClientGetReq with Size
        int size = 0;
        socketHandler_receiveCliGetReq(&size);
        printf("Size to alloc: %d bytes\n", size);
        char* reqBuf = malloc(size);

        // Send ACK
        socketHandler_sendAck();

        // Receive JSON data
        socketHandler_receiveJson(&reqBuf, size);
        printf("Received JSON: %s\n", reqBuf);

        // Parse client request JSON
        cJSON* cliReqJson = cJSON_Parse(reqBuf);
        int id = 0;
        OSS_Pioj(&id, cliReqJson, "id");
        printf("Action ID is %d\n", id);

        // Perform action
        char* retData = NULL;
        socketHandler_performAction(id, &retData, &cliReqJson);
        int retDataSize = strlen(retData);

        // Send size back to client
        socketHandler_sendSize(retDataSize);

        // Wait for ACK
        socketHandler_receiveAck();

        // Send JSON
        socketHandler_sendJson(retData, retDataSize);

        // Wait for ACK
        socketHandler_receiveAck();
    }
}

void socketHandler_cleanup() {
    printf("[SocketHandler] Cleaning up.\n");

    close(client_fd);
    close(server_fd);
    unlink(configObj->client_socket_path);
}

void socketHandler_performAction(int id, char** retDataStr, cJSON** cliReqJson) {
    switch (id) {
        case OSSP_SOCKET_ACTION_GET_STARRED_SONGS:
            printf("[SocketHandler] Client requested OSSP_SOCKET_ACTION_GET_STARRED_SONGS.\n");
            OSSPS_SocketAction_Get_Starred_Songs(retDataStr, cliReqJson);
            break;
        case OSSP_SOCKET_ACTION_NOW_PLAYING:
            printf("[SocketHandler] Client requested OSSP_SOCKET_ACTION_NOW_PLAYING.\n");
            OSSPS_SocketAction_Now_Playing(retDataStr, cliReqJson);
            break;
        case OSSP_SOCKET_ACTION_ADD_TO_QUEUE:
            printf("[SocketHandler] Client requested OSSP_SOCKET_ACTION_ADD_TO_QUEUE.\n");
            OSSPS_SocketAction_Add_To_Queue(retDataStr, cliReqJson);
            break;



        case OSSP_SOCKET_ACTION_OSSPP_PREV:
            OSSPlayer_GstECont_Playbin3_Prev();
            *retDataStr = strdup("OK");
            break;
        case OSSP_SOCKET_ACTION_OSSPP_PLAYPAUSE:
            OSSPlayer_GstECont_Playbin3_PlayPause();
            *retDataStr = strdup("OK");
            break;
        case OSSP_SOCKET_ACTION_OSSPP_NEXT:
            OSSPlayer_GstECont_Playbin3_Next();
            *retDataStr = strdup("OK");
            break;
        case OSSP_SOCKET_ACTION_OSSPP_OUTVOLUME:
            // New OutVolume set from client
            printf("[SocketHandler] Client requested OSSP_SOCKET_ACTION_OSSPP_OUTVOLUME.\n");
            OSSPS_SocketAction_OSSPP_OutVolume(retDataStr, cliReqJson);
            break;
        case OSSP_SOCKET_ACTION_OSSPP_INVOLUME:
            // New InVolume set from client
            printf("[SocketHandler] Client requested OSSP_SOCKET_ACTION_OSSPP_INVOLUME.\n");
            OSSPS_SocketAction_OSSPP_InVolume(retDataStr, cliReqJson);
            break;



        default:
            printf("[SocketHandler] Unknown action.\n");
            break;
    }
}















int socketHandler_initClientConnection() {
    /*
     * - Wait for client to send OSSP_Sock_CliConn
     * - Send OSSP_Sock_ACK back
     * - Wait for client to send OSSP_Sock_GetConnInfo
     * - Send back OSSP_Sock_Size
     * - Wait for client to send OSSP_Sock_ACK
     * - Send JSON
     * - Wait for client to send OSSP_Sock_ACK
     */

    if (socketHandler_receiveCliConn() != 0) {
        printf("[SocketHandler] initClientConnection() failed.\n");
        return 1;
    }

    if (socketHandler_sendAck() != 0) {
        printf("[SocketHandler] initClientConnection() failed.\n");
        return 1;
    }

    if (socketHandler_receiveGetConnInfo() != 0) {
        printf("[SockerHandler] initClientConnection() failed.\n");
        return 1;
    }

    // Form JSON of connection info
    char* serverAddr = NULL;
    rc = asprintf(&serverAddr, "%s://%s", configObj->opensubsonic_protocol, configObj->opensubsonic_server);
    if (rc == -1) {
        printf("[SocketHandler] asprintf() failed.\n");
        return 1;
    }

    cJSON* connInfoObj = cJSON_CreateObject();
    cJSON_AddItemToObject(connInfoObj, "ossp_version", cJSON_CreateString(configObj->internal_ossp_version));
    cJSON_AddItemToObject(connInfoObj, "server_addr", cJSON_CreateString(serverAddr));
    char* connInfoStr = cJSON_PrintUnformatted(connInfoObj);
    int connInfoLen = strlen(connInfoStr);
    free(serverAddr);
    cJSON_Delete(connInfoObj);

    if (socketHandler_sendSize(connInfoLen) != 0) {
        printf("[SockerHandler] initClientConnection() failed.\n");
        return 1;
    }

    if (socketHandler_receiveAck() != 0) {
        printf("[SockerHandler] initClientConnection() failed.\n");
        return 1;
    }

    if (socketHandler_sendJson(connInfoStr, connInfoLen) != 0) {
        printf("[SockerHandler] initClientConnection() failed.\n");
        return 1;
    }

    if (socketHandler_receiveAck() != 0) {
        printf("[SockerHandler] initClientConnection() failed.\n");
        return 1;
    }

    return 0;
}




int socketHandler_sendAck() {
    printf("[SocketHandler] Sending OSSP_Sock_ACK.\n");

    rc = send(client_fd, &OSSP_Sock_ACK, SockSig_Length, 0);
    if (rc != SockSig_Length) {
        printf("[SocketHandler] Failed to send OSSP_Sock_ACK.\n");
        return 1;
    }
    return 0;
}

int socketHandler_receiveAck() {
    uint8_t buf[4] = { 0x00 };
    rc = read(client_fd, buf, SockSig_Length);
    if (rc != SockSig_Length) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_ACK (Signature is the wrong length).\n");
        return 1;
    }

    uint32_t AckLE = socketHandlerUtil_byteArrToUint32LE(buf);

    rc = memcmp(&AckLE, &OSSP_Sock_ACK, SockSig_Length);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_ACK (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_ACK, AckLE);
        return 1;
    }
    printf("[SocketHandler] Received OSSP_Sock_ACK.\n");
    return 0;
}



int socketHandler_receiveFragMsgAck() {
    uint8_t buf[4] = { 0x00 };
    rc = read(client_fd, buf, SockSig_Length);
    if (rc != SockSig_Length) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_FragMsgACK (Signature is the wrong length).\n");
        return 1;
    }

    uint32_t AckLE = socketHandlerUtil_byteArrToUint32LE(buf);

    rc = memcmp(&AckLE, &OSSP_Sock_FragMsgACK, SockSig_Length);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_FragMsgACK (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_FragMsgACK, AckLE);
        return 1;
    }
    printf("[SocketHandler] Received OSSP_Sock_FragMsgACK.\n");
    return 0;
}



int socketHandler_receiveCliConn() {
    char buf[4] = { 0x00 };
    rc = read(client_fd, buf, SockSig_Length);
    if (rc != SockSig_Length) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_CliConn (Signature is the wrong length).\n");
        return 1;
    }

    uint32_t CliConnLE = socketHandlerUtil_byteArrToUint32LE(buf);

    rc = memcmp(&CliConnLE, &OSSP_Sock_CliConn, SockSig_Length);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_CliConn (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_CliConn, CliConnLE);
        return 1;
    }
    printf("[SocketHandler] Received OSSP_Sock_CliConn.\n");
    return 0;
}



int socketHandler_receiveGetConnInfo() {
    char buf[4] = { 0x00 };
    rc = read(client_fd, buf, SockSig_Length);
    if (rc != SockSig_Length) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_GetConnInfo (Signature is the wrong length).\n");
        return 1;
    }

    uint32_t GetConnInfoLE = socketHandlerUtil_byteArrToUint32LE(buf);

    rc = memcmp(&GetConnInfoLE, &OSSP_Sock_GetConnInfo, SockSig_Length);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_GetConnInfo (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_GetConnInfo, GetConnInfoLE);
        return 1;
    }
    printf("[SocketHandler] Received OSSP_Sock_GetConnInfo.\n");
    return 0;
}










int socketHandler_sendSize(uint32_t size) {
    printf("[SocketHandler] Sending OSSP_Sock_Size.\n");

    OSSP_Sock_Size_t sizeData;
    sizeData.signature = OSSP_Sock_Size;
    sizeData.size = size;

    rc = send(client_fd, (char*)&sizeData, 8, 0);
    if (rc != 8) {
        printf("[SocketHandler] Failed to send OSSP_Sock_Size.\n");
        return 1;
    }
    return 0;
}

int socketHandler_receiveSize(uint32_t* size) {
    char buf[8] = { 0x00 };
    rc = read(client_fd, buf, 8);
    if (rc != 8) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_Size (Invalid size).\n");
        return 1;
    }

    OSSP_Sock_Size_t* sizeData = (OSSP_Sock_Size_t*)&buf;

    rc = memcmp(&sizeData->signature, &OSSP_Sock_Size, 4);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_Size (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_Size, sizeData->signature);
        return 1;
    }

    printf("[SocketHandler] Received OSSP_Sock_Size.\n");
    *size = sizeData->size;
    return 0;
}


int socketHandler_receiveCliGetReq(int* size) {
    uint8_t buf[8] = { 0x00 };
    rc = read(client_fd, buf, 8);
    if (rc != 8) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_ClientGetReq (Invalid size).\n");
        return 1;
    }

    OSSP_Sock_ClientGetReq_t* clientGetReq = (OSSP_Sock_ClientGetReq_t*)&buf;
    
    rc = memcmp(&clientGetReq->signature, &OSSP_Sock_ClientGetReq, 4);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_ClientGetReq (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_ClientGetReq, clientGetReq->signature);
        return 1;
    }

    printf("[SocketHandler] Received OSSP_Sock_ClientGetReq.\n");
    *size = clientGetReq->size;
    return 0;
}

int socketHandler_receiveJson(char** data, int size) {
    rc = read(client_fd, *data, size);
    if (rc != size) {
        printf("[SocketHandler] Failed to receive generic JSON data.\n");
        return 1;
    }

    printf("[SocketHandler] Received generic JSON data.\n");
    return 0;
}

int socketHandler_sendJson(char* json, int size) {
    printf("[SocketHandler] Sending JSON.\n");
    
    /*
     * Notes on IPC messaging sizes and fragmented sending
     * - Able to get macOS max IPC message size with sysctl kern.ipc.maxsockbuf
     * - Sticking with 8192 bytes for safety
     * - IPC messages REQUIRES contiguous memory
     */

    float msgCount_intermediate = (float)size / 8192;
    int msgCount = (int)ceil(msgCount_intermediate);
    int msgCount_remainer = size % 8192; // If using fragmented messaging, find remaining bytes in last message

    if (msgCount > 1) {
        // Fragmented messaging required
        printf("[SocketHandler] Sending fragmented payload in %d chunks.\n", msgCount);
        for (int i = 0; i < msgCount; i++) {
            printf("[SocketHandler] Sending fragment %d.\n", i);

            if (i == msgCount - 1) {
                // Last fragment, use remainder bytes as message length
                rc = send(client_fd, json + (i * 8192), msgCount_remainer, 0);
                if (rc != msgCount_remainer) {
                    printf("[SocketHandler] Failed to send JSON.\n");
                    return 1;
                }
            } else {
                // Use 8192 bytes as the message length
                rc = send(client_fd, json + (i * 8192), 8192, 0);
                if (rc != 8192) {
                    printf("[SocketHandler] Failed to send JSON.\n");
                    return 1;
                }
            }

            // Wait for OSSP_Sock_FragMsgACK
            socketHandler_receiveFragMsgAck();
        }
    } else {
        // Payload fits inside single message
        printf("[SocketHandler] Sending non-fragmented payload.\n");
        rc = send(client_fd, json, size, 0);
        if (rc != size) {
            printf("[SocketHandler] Failed to send JSON.\n");
            return 1;
        }
    }

    return 0;
}






int socketHandler_processClientGetReq() {
    // Step 1 - Client sends GetReq with size

    // Step 2 - Server allocates memory for future client request

    // Step 3 - Server responds ACK

    // Step 4 - --
}


/*
 * Utility Functions
 */
// Byte Array to uint32_t Big Endian
uint32_t socketHandlerUtil_byteArrToUint32BE(uint8_t buf[]) {
    // NOTE: I could use a combination of memcpy() and htons() here, but bitshifting is a single move
    uint32_t retVal = 0x0;
    retVal = buf[3] | buf[2] << 8 | buf[1] << 16 | buf[0] << 24;
    return retVal;
}

// Byte Array to uint32_t Little Endian
uint32_t socketHandlerUtil_byteArrToUint32LE(uint8_t buf[]) {
    uint32_t retVal = 0x0;
    retVal = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
    return retVal;
}
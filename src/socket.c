/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Socket Handler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "external/cJSON.h"
#include "configHandler.h"
#include "socket.h"

#define SOCKET_PATH "/tmp/ossp_socket" // TODO Make this configurable through the configuration file
static int server_fd = -1;
static int client_fd = -1;
static int rc = -1;
socklen_t client_len;
struct sockaddr_un server_addr;
struct sockaddr_un client_addr;
extern configHandler_config_t* configObj;



void socketHandler_read();

int SockSig_Length = 4;

const uint32_t OSSP_Sock_ACK = 0x7253FF87;
const uint32_t OSSP_Sock_CliConn = 0xE3566C2E;
const uint32_t OSSP_Sock_GetConnInfo = 0x8E4F6B01;
const uint32_t OSSP_Sock_Size = 0x1F7E8BCF;

const uint32_t OSSP_Sock_ClientGetReq = 0x210829CF;



void socket_setup() {
    printf("[SocketHandler] Initializing.\n");

    // Create server socket, and ensure that the socket file is removed
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("[SocketHandler] Could not open server socket.\n");
        // TODO
    }

    unlink(SOCKET_PATH);

    // Bind server socket to SOCKET_PATH
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

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


    //socketHandler_read();
    socketHandler_initClientConnection();



    printf("------------------------------\n");


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
}

void socketHandler_cleanup() {
    printf("[SocketHandler] Cleaning up.\n");

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
}






// Byte Array to uint32_t Big Endian
uint32_t util_byteArrToUint32BE(char buf[]) {
    // NOTE: I could use a combination of memcpy() and htons() here, but bitshifting is a single move
    uint32_t retVal = 0x0;
    retVal = buf[3] | buf[2] << 8 | buf[1] << 16 | buf[0] << 24;
    return retVal;
}

// Byte Array to uint32_t Little Endian
uint32_t util_byteArrToUint32LE(char buf[]) {
    uint32_t retVal = 0x0;
    retVal = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
    return retVal;
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
    char buf[4] = { 0x00 };
    rc = read(client_fd, buf, SockSig_Length);
    if (rc != SockSig_Length) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_ACK (Signature is the wrong length).\n");
        return 1;
    }

    uint32_t AckBE = util_byteArrToUint32LE(buf);

    rc = memcmp(&AckBE, &OSSP_Sock_ACK, SockSig_Length);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_ACK (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_ACK, AckBE);
        return 1;
    }
    printf("[SocketHandler] Received OSSP_Sock_ACK.\n");
    return 0;
}



int socketHandler_receiveCliConn() {
    char buf[4] = { 0x00 };
    rc = read(client_fd, buf, SockSig_Length);
    if (rc != SockSig_Length) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_CliConn (Signature is the wrong length).\n");
        return 1;
    }

    //uint32_t CliConnBE = util_byteArrToUint32BE(buf);
    uint32_t CliConnBE = util_byteArrToUint32LE(buf);

    rc = memcmp(&CliConnBE, &OSSP_Sock_CliConn, SockSig_Length);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_CliConn (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_CliConn, CliConnBE);
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

    uint32_t GetConnInfoBE = util_byteArrToUint32LE(buf);

    rc = memcmp(&GetConnInfoBE, &OSSP_Sock_GetConnInfo, SockSig_Length);
    if (rc != 0) {
        printf("[SocketHandler] Failed to receive OSSP_Sock_GetConnInfo (Signature is invalid. Expected 0x%.8x, Received 0x%.8x).\n", OSSP_Sock_GetConnInfo, GetConnInfoBE);
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
    char buf[8] = { 0x00 };
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

    rc = send(client_fd, json, size, 0);
    if (rc != size) {
        printf("[SocketHandler] Failed to send JSON.\n");
        return 1;
    }
    return 0;
}






int socketHandler_processClientGetReq() {
    // Step 1 - Client sends GetReq with size

    // Step 2 - Server allocates memory for future client request

    // Step 3 - Server responds ACK

    // Step 4 - --
}

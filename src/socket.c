/*
 * OpenSubsonicPlayer
 * Goldenkrew3000 2025
 * License: GNU General Public License 3.0
 * Info: Socket Handler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "socket.h"

#define SOCKET_PATH "/tmp/ossp_socket" // TODO Make this configurable through the configuration file
static int server_fd = -1;
static int client_fd = -1;
static int rc = -1;
socklen_t client_len;
struct sockaddr_un server_addr;
struct sockaddr_un client_addr;



void socketHandler_read();
void socketHandler_initClientConnection();
void socketHandler_getConnInfo();
#define SOCKSIG_CLIENT_CONNECT      0xE3566C2E
const uint32_t SockSig_Client_Connect = 0xE3566C2E;
#define SOCKSIG_SIZE                0x1F7E8BCF
#define SOCKSIG_ACK                 0x7253FF87
const uint32_t SockSig_Ack = 0x7253FF87;
const uint32_t SockSig_GetConnInfo = 0x8E4F6B01;
const uint32_t SockSig_Size = 0x1F7E8BCF;


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



    //char buffer[256];
    //client_len = sizeof(struct sockaddr_un);
    //read(client_fd, buffer, 256);
    //printf("Received: %s\n", buffer);
    //printf("Client len: %d\n", client_len);

    // Cleanup
    //close(client_fd);
    //close(server_fd);
    //unlink(SOCKET_PATH);
}

void socketHandler_cleanup() {
    printf("[SocketHandler] Cleaning up.\n");

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
}

void socketHandler_read() {
    char buf[16];
    int buflen = 16;
    for (int i = 0; i < buflen; i++) { buf[i] = 0x00; }
    rc = read(client_fd, buf, buflen);
    printf("Read %d: %s\n", buflen, buf);
    printf("RC was %d\n", rc);
}

// Byte Array to uint32_t Big Endian
uint32_t util_byteArrToUint32BE(char buf[]) {
    // NOTE: I could use a combination of memcpy() and htons() here, but bitshifting is a single move
    uint32_t retVal = 0x0;
    retVal = buf[3] | buf[2] << 8 | buf[1] << 16 | buf[0] << 24;
    return retVal;
}

void socketHandler_initClientConnection() {
    // If reached here, a client has connected to the AF_UNIX socket

    // Receive CliConn
    int cliConnSize = 4;
    char buf[4] = { 0x00 };
    rc = read(client_fd, buf, cliConnSize);
    if (rc != cliConnSize) {
        printf("Received data was not correct.\n");
    }
    uint32_t cliConn = util_byteArrToUint32BE(buf);
    rc = memcmp(&cliConn, &SockSig_Client_Connect, 4);
    if (rc != 0) {
        printf("Received CliConn signature is not valid.\n");
        // TODO
    } else {
        printf("Client connected!\n");
    }

    // Send Server ACK
    rc = send(client_fd, &SockSig_Ack, 4, 0);
    if (rc != 4) {
        printf("Failed to send Server ACK.\n");
    } else {
        printf("Sent Server ACK.\n");
    }

    // Deal with connection info
    socketHandler_getConnInfo();
}

typedef struct {
    uint32_t signature;
    uint16_t size;
} __attribute__((packed)) OSSP_Sock_Size_Data;

void socketHandler_getConnInfo() {
    // Receive OSSP_Sock_GetConnInfo
    int sigSize = 4;
    char buf[4] = { 0x00 };
    rc = read(client_fd, buf, sigSize);
    if (rc != sigSize) {
        printf("OSSP_Sock_GetConnInfo Error 1\n");
        // TODO
    } else {
        printf("Signature verified.\n");
    }

    // Assemble JSON
    char* jsonInfo = "{ 'ossp_version': '0.3a', 'server_addr': 'https://eumak.hojuix.org' }";
    int jsonLen = strlen(jsonInfo);
    printf("JSON: %s\n", jsonInfo);
    printf("JSON Length: %d\n", jsonLen);

    // Send OSSP_Sock_Size
    OSSP_Sock_Size_Data ossp_sock;
    ossp_sock.signature = SockSig_Size;
    ossp_sock.size = jsonLen;
    int sizeLen = 6;
    rc = send(client_fd, (char*)&ossp_sock, sizeLen, 0);
    if (rc != sizeLen) {
        printf("OSSP_Sock_Size failed.\n");
    } else { printf("OSSP_Sock_Size sent.\n"); }


    // Wait for ACK
    char bufb[4] = { 0x00 };
    rc = read(client_fd, bufb, sigSize);
    if (rc != sigSize) {
        printf("Invalid ACK sig size");
    } else { printf("Sig verified\n"); }

    // Send JSON
    rc = send(client_fd, jsonInfo, jsonLen, 0);
    if (rc != jsonLen) {
        printf("Failed to send JSON.\n");
    } else { printf("Sent JSON.\n"); }
}

void sockerHandler_sendAck() {
    // Send OSSP_Sock_ACK to the client
}

void socketHandler_receiveAck() {
    // Receive OSSP_Sock_ACK from the client
}

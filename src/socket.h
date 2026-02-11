#ifndef _SOCKET_H
#define _SOCKET_H
#include <stdint.h>

typedef struct {
    uint32_t signature;
    uint32_t size;
} __attribute__((packed)) OSSP_Sock_Size_t;

typedef struct {
    uint32_t signature;
    uint32_t size;
} __attribute__((packed)) OSSP_Sock_ClientGetReq_t;

void socket_setup();
int socketHandler_initClientConnection();

int socketHandler_sendAck();
int socketHandler_receiveAck();
int socketHandler_receiveCliConn();
int socketHandler_receiveGetConnInfo();


int socketHandler_sendSize(uint32_t size);
int socketHandler_receiveSize(uint32_t* size);
int socketHandler_receiveCliGetReq(int* size);
int socketHandler_receiveJson(char** data, int size);
int socketHandler_sendJson(char* json, int size);

#endif

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

int socketHandler_init();
void socketHandler_cleanup();
int socketHandler_initClientConnection();

int socketHandler_sendAck();
int socketHandler_receiveAck();
int socketHandler_receiveFragMsgAck();
int socketHandler_receiveCliConn();
int socketHandler_receiveGetConnInfo();


int socketHandler_sendSize(uint32_t size);
int socketHandler_receiveSize(uint32_t* size);
int socketHandler_receiveCliGetReq(int* size);
int socketHandler_receiveJson(char** data, int size);
int socketHandler_sendJson(char* json, int size);

uint32_t socketHandlerUtil_byteArrToUint32BE(uint8_t buf[]);
uint32_t socketHandlerUtil_byteArrToUint32LE(uint8_t buf[]);

#endif

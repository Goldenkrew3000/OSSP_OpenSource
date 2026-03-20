#ifndef _SOCKET_H
#define _SOCKET_H
#include <stdint.h>
#include "external/cJSON.h"

#define OSSP_SOCKET_ACTION_GETSTARREDSONGS 101
#define OSSP_SOCKET_ACTION_NOW_PLAYING 201
#define OSSP_SOCKET_ACTION_STATS 202
#define OSSP_SOCKET_ACTION_ADD_TO_QUEUE 203

#define OSSP_SOCKET_ACTION_OSSPP_PREV 301       // (OSSPP -> OSSP Player)
#define OSSP_SOCKET_ACTION_OSSPP_PLAYPAUSE 302
#define OSSP_SOCKET_ACTION_OSSPP_NEXT 303
#define OSSP_SOCKET_ACTION_OSSPP_OUTVOLUME 304

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



// Testing
void OSSPS_SocketAction_Add_To_Queue(char** retDataStr, cJSON** cliReqJson);

#endif

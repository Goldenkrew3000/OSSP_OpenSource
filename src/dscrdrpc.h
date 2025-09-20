#ifndef _DSCRDRPC_H
#define _DSCRDRPC_H
#include <openssl/evp.h>

#define DSCRDRPC_REQTYPE_PLAYING 1
#define DSCRDRPC_REQTYPE_PAUSED 2
#define DSCRDRPC_REQTYPE_UNPAUSED 3
#define DSCRDRPC_REQTYPE_CLEAR 4
#define DSCRDRPC_REQTYPE_KEEPALIVE 5

typedef struct {
    int requestType;
    char* trackingUuid;
    long songLength;
    char* songTitle;
    char* songArtist;
    char* coverArtUrl;
    char* deviceInfo;
    char* checksum;
} dscrdrpc_data;

void dscrdrpc_struct_init(dscrdrpc_data** dscrdrpc_struct);
void dscrdrpc_struct_deinit(dscrdrpc_data** dscrdrpc_struct);
int dscrdrpc_uuidv4(char** uuidv4);
int dscrdrpc_crc32(char* inputString, char** crc32Output);
void dscrdrpc_form_innerJSON(dscrdrpc_data** dscrdrpc_struct, char** jsonOutput);
void dscrdrpc_aes_gcm_encrypt(unsigned char* key, unsigned char* iv, unsigned char* ct, unsigned char* tag, char* plaintext, int length);
EVP_PKEY* dscrdrpc_rsa_read_pubkey(char* pubkey);
int dscrdrpc_rsa_oaep_encrypt(EVP_PKEY* pkey, char* plaintext, int length, char** ct);
void dscrdrpc_form_outerJSON(char* iv, char* tag, char* ct, char* checksum, char** jsonOutput);

int dscrdrpc_encrypt(dscrdrpc_data** dscrdrpc_struct);

#endif

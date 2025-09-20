/*
 * NOTE: The private keys in this file are PURELY for FORMATTING DEMONSTRATION. They are NOT in active use
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include "external/cJSON.h"
#include "external/libcurl_uriescape.h"
#include "dscrdrpc.h"

#include "DarwinHttpClient.h"

void dscrdrpc_struct_init(dscrdrpc_data** dscrdrpc_struct) {
    (*dscrdrpc_struct) = (dscrdrpc_data*)malloc(sizeof(dscrdrpc_data));
    (*dscrdrpc_struct)->requestType = 0;
    (*dscrdrpc_struct)->trackingUuid = NULL;
    (*dscrdrpc_struct)->songLength = 0;
    (*dscrdrpc_struct)->songTitle = NULL;
    (*dscrdrpc_struct)->songArtist = NULL;
    (*dscrdrpc_struct)->coverArtUrl = NULL;
    (*dscrdrpc_struct)->deviceInfo = NULL;
    (*dscrdrpc_struct)->checksum = NULL;
}

void dscrdrpc_struct_deinit(dscrdrpc_data** dscrdrpc_struct) {
    if ((*dscrdrpc_struct)->trackingUuid != NULL) { free((*dscrdrpc_struct)->trackingUuid); }
    if ((*dscrdrpc_struct)->songTitle != NULL) { free((*dscrdrpc_struct)->songTitle); }
    if ((*dscrdrpc_struct)->songArtist != NULL) { free((*dscrdrpc_struct)->songArtist); }
    if ((*dscrdrpc_struct)->coverArtUrl != NULL) { free((*dscrdrpc_struct)->coverArtUrl); }
    if ((*dscrdrpc_struct)->deviceInfo != NULL) { free((*dscrdrpc_struct)->deviceInfo); }
    if ((*dscrdrpc_struct)->checksum != NULL) { free((*dscrdrpc_struct)->checksum); }
    if (*dscrdrpc_struct != NULL) { free(*dscrdrpc_struct); }
}

int dscrdrpc_uuidv4(char** uuidv4) {
    // Generate non-compliant UUIDv4 string
    uint8_t uuidv4_bytes[16];
    static int rc = 0;
    for (int i = 0; i < 16; i++) {
        uuidv4_bytes[i] = arc4random() & 0xFF;
    }
    rc = asprintf(uuidv4, "%.2x%.2x%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x",
                  uuidv4_bytes[0], uuidv4_bytes[1], uuidv4_bytes[2], uuidv4_bytes[3], uuidv4_bytes[4],
                  uuidv4_bytes[5], uuidv4_bytes[6], uuidv4_bytes[7], uuidv4_bytes[8], uuidv4_bytes[9],
                  uuidv4_bytes[10], uuidv4_bytes[11], uuidv4_bytes[12], uuidv4_bytes[13], uuidv4_bytes[14],
                  uuidv4_bytes[15]);
    if (rc == -1) {
        printf("asprintf() failed.\n");
        return 1;
    }
    return 0;
}

int dscrdrpc_crc32(char* inputString, char** crc32Output) {
    static int rc = 0;
    uLong crc = crc32(0, (void*)inputString, (uInt)strlen(inputString));
    rc = asprintf(crc32Output, "%lx", crc);
    if (rc == -1) {
        printf("asprintf() failed.\n");
        return 1;
    }
    return 0;
}

void dscrdrpc_form_innerJSON(dscrdrpc_data** dscrdrpc_struct, char** jsonOutput) {
    cJSON* internal_root = cJSON_CreateObject();
    cJSON* internal_requestType = cJSON_CreateNumber((*dscrdrpc_struct)->requestType);
    cJSON* internal_trackingUuid = cJSON_CreateString((*dscrdrpc_struct)->trackingUuid);
    cJSON* internal_songLength = cJSON_CreateNumber((*dscrdrpc_struct)->songLength);
    cJSON* internal_songTitle = cJSON_CreateString((*dscrdrpc_struct)->songTitle);
    cJSON* internal_songArtist = cJSON_CreateString((*dscrdrpc_struct)->songArtist);
    cJSON* internal_coverArtUrl = cJSON_CreateString((*dscrdrpc_struct)->coverArtUrl);
    cJSON* internal_deviceInfo = cJSON_CreateString((*dscrdrpc_struct)->deviceInfo);
    cJSON* internal_checksum = cJSON_CreateString((*dscrdrpc_struct)->checksum);
    cJSON_AddItemToObject(internal_root, "requestType", internal_requestType);
    cJSON_AddItemToObject(internal_root, "trackingUuid", internal_trackingUuid);
    cJSON_AddItemToObject(internal_root, "songLength", internal_songLength);
    cJSON_AddItemToObject(internal_root, "songTitle", internal_songTitle);
    cJSON_AddItemToObject(internal_root, "songArtist", internal_songArtist);
    cJSON_AddItemToObject(internal_root, "coverArtUrl", internal_coverArtUrl);
    cJSON_AddItemToObject(internal_root, "deviceInfo", internal_deviceInfo);
    cJSON_AddItemToObject(internal_root, "checksum", internal_checksum);
    *jsonOutput = cJSON_PrintUnformatted(internal_root);
    cJSON_Delete(internal_root);
}

void dscrdrpc_aes_gcm_encrypt(unsigned char* key, unsigned char* iv, unsigned char* ct, unsigned char* tag,
                              char* plaintext, int length) {
    // Initialize OpenSSL
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    
    EVP_EncryptInit(ctx, EVP_aes_256_gcm(), NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL); // 96 bit IV
    EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
    
    // Encrypt the plaintext
    int len;
    EVP_EncryptUpdate(ctx, ct, &len, (const unsigned char*)plaintext, length);
    EVP_EncryptFinal(ctx, ct + len, &len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
    
    // Cleanup
    EVP_CIPHER_CTX_free(ctx);
}

EVP_PKEY* dscrdrpc_rsa_read_pubkey(char* pubkey) {
    BIO* bio = BIO_new_mem_buf(pubkey, -1); // -1 causes strlen to be called
    if (!bio) {
        printf("BIO_new_mem_buf() failed.\n");
        return NULL;
    }
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (!pkey) {
        printf("PEM_read_bio_RSA_PUBKEY() failed.\n");
        return NULL;
    }
    return pkey;
}

int dscrdrpc_rsa_oaep_encrypt(EVP_PKEY* pkey, char* plaintext, int length, char** ct) {
    // Initialize OpenSSL
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    EVP_PKEY_encrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);
    EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256());
    EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256());
    
    // Encrypt
    size_t len;
    EVP_PKEY_encrypt(ctx, NULL, &len, (const unsigned char*)plaintext, length);
    *ct = malloc(len);
    EVP_PKEY_encrypt(ctx, (unsigned char*)*ct, &len, (const unsigned char*)plaintext, length);
    
    // Cleanup
    EVP_PKEY_CTX_free(ctx);
    return (int)len;
}

void dscrdrpc_form_outerJSON(char* iv, char* tag, char* ct, char* checksum, char** jsonOutput) {
    cJSON* outer_root = cJSON_CreateObject();
    cJSON* outer_iv = cJSON_CreateString(iv);
    cJSON* outer_tag = cJSON_CreateString(tag);
    cJSON* outer_ct = cJSON_CreateString(ct);
    cJSON* outer_checksum = cJSON_CreateString(checksum);
    cJSON_AddItemToObject(outer_root, "i", outer_iv);
    cJSON_AddItemToObject(outer_root, "t", outer_tag);
    cJSON_AddItemToObject(outer_root, "d", outer_ct);
    cJSON_AddItemToObject(outer_root, "c", outer_checksum);
    *jsonOutput = cJSON_PrintUnformatted(outer_root);
    cJSON_Delete(outer_root);
}

int dscrdrpc_encrypt(dscrdrpc_data** dscrdrpc_struct) {
    static int rc = 0;
    
    // Generate UUIDv4 (Non compliant but does not matter)
    dscrdrpc_uuidv4(&(*dscrdrpc_struct)->trackingUuid);
    
    // Form string to make checksum
    char* innerContentChecksumStr = NULL;
    rc = asprintf(&innerContentChecksumStr, "%d%s%s%ld%s%s%s",
                  (*dscrdrpc_struct)->requestType, (*dscrdrpc_struct)->songTitle, (*dscrdrpc_struct)->songArtist,
                  (*dscrdrpc_struct)->songLength, (*dscrdrpc_struct)->coverArtUrl, (*dscrdrpc_struct)->deviceInfo,
                  (*dscrdrpc_struct)->trackingUuid);
    if (rc == -1) {
        printf("asprintf() failed.\n");
        return 1;
    }
    
    // Create CRC32 checksum of contents
    dscrdrpc_crc32(innerContentChecksumStr, &(*dscrdrpc_struct)->checksum);
    free(innerContentChecksumStr);
    
    // Form inner JSON
    char* innerJSON = NULL;
    dscrdrpc_form_innerJSON(dscrdrpc_struct, &innerJSON);
    int innerJSON_length = (int)strlen(innerJSON);
    
    // Encrypt inner JSON with AES GCM
    unsigned char aes_key[32] = {
        0x43, 0x63, 0x50, 0x5d, 0x31, 0x23, 0x46, 0x51,
        0x74, 0x50, 0x70, 0x55, 0x4d, 0x60, 0x46, 0x69,
        0x39, 0x46, 0x52, 0x6e, 0x5f, 0x5e, 0x2f, 0x50,
        0x45, 0x27, 0x30, 0x55, 0x39, 0x68, 0x78, 0x43
    };
    unsigned char* aes_iv = calloc(12, sizeof(char));
    RAND_bytes(aes_iv, 12); // TODO replace with arc4random
    unsigned char* ciphertext = calloc(innerJSON_length, sizeof(char));
    unsigned char* aes_tag = calloc(16, sizeof(char));
    dscrdrpc_aes_gcm_encrypt(aes_key, aes_iv, ciphertext, aes_tag, innerJSON, innerJSON_length);
    free(innerJSON);
    
    printf("Key: "); for (int i = 0; i < 32; i++) { printf("%.2x", aes_key[i]); } printf("\n");
    printf("IV: "); for (int i = 0; i < 12; i++) { printf("%.2x", aes_iv[i]); } printf("\n");
    printf("Tag: "); for (int i = 0; i < 16; i++) { printf("%.2x", aes_tag[i]); } printf("\n");
    printf("Ciphertext: "); for (int i = 0; i < innerJSON_length; i++) { printf("%.2x", ciphertext[i]); } printf("\n");
    
    // RSA encrypt the AES IV and Tag
    char* rsa_pubkey_text = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAoeZn81EnEGom5eWOpFvB\nZw3X9gDBtjzix69qhWDHfq2oh/b0tynIe2PV7G6ELr2StwQOzcVa4cx3HkQ4vmE8\nZxdQ3Ru+HN6EjGonXrfgN7J6+MUDcEE0wOglAkoIGyrDhuxlMrUFKwiTVMdPqxQy\ngOWbOvnJu0q5x/7TFbJGgkZwdRYHgFXW/1lTEzBfZZu0cCa0V7YQ0prCxtbGP5CD\nXaDND65a/rk5t4Jp+3nOQDUtd6tqZ9Rp/mOBRQm8bj4nhw03UsbRhV+vHx34rtPl\nw46JSUfEuGXiNNaluXbPxWPNAf6Lj36UJH01tFVnJrPtv6T5mQCygL22OgkbSsAN\n3QIDAQAB\n-----END PUBLIC KEY-----";
    EVP_PKEY* rsa_pubkey = dscrdrpc_rsa_read_pubkey(rsa_pubkey_text);
    
    char* ret_rsa_iv = NULL;
    char* ret_rsa_tag = NULL;
    int ret_rsa_iv_len = dscrdrpc_rsa_oaep_encrypt(rsa_pubkey, (char*)aes_iv, 12, &ret_rsa_iv);
    int ret_rsa_tag_len = dscrdrpc_rsa_oaep_encrypt(rsa_pubkey, (char*)aes_tag, 16, &ret_rsa_tag);
    free(aes_iv);
    free(aes_tag);
    
    char* escaped_iv = lcue_uriescape(ret_rsa_iv, ret_rsa_iv_len);
    char* escaped_tag = lcue_uriescape(ret_rsa_tag, ret_rsa_tag_len);
    char* escaped_ciphertext = lcue_uriescape((char*)ciphertext, innerJSON_length);
    char* escaped_aes_key = lcue_uriescape((char*)aes_key, sizeof(aes_key));
    
    free(ciphertext);
    free(ret_rsa_iv);
    free(ret_rsa_tag);
    EVP_PKEY_free(rsa_pubkey);
    
    // Form string to make checksum
    char* outerContentChecksumStr = NULL;
    rc = asprintf(&outerContentChecksumStr, "%s%s%s%s",
                  escaped_ciphertext, escaped_iv, escaped_tag, escaped_aes_key);
    if (rc == -1) {
        printf("asprintf() failed.\n");
        return 1;
    }
    
    // Create CRC32 checksum of contents
    char* outerContentChecksum = NULL;
    dscrdrpc_crc32(outerContentChecksumStr, &outerContentChecksum);
    free(outerContentChecksumStr);
    
    // Form Outer JSON
    char* finalPayload = NULL;
    dscrdrpc_form_outerJSON(escaped_iv, escaped_tag, escaped_ciphertext, outerContentChecksum, &finalPayload);
    
    free(escaped_iv);
    free(escaped_tag);
    free(escaped_ciphertext);
    free(escaped_aes_key);
    free(outerContentChecksum);
    
    printf("Final payload: %s\n", finalPayload);
    
    opensubsonic_httpClientRequest_t* httpReq;
    opensubsonic_httpClient_prepareRequest(&httpReq);
    httpReq->method = HTTP_METHOD_POST;
    httpReq->requestUrl = strdup("http://192.168.5.140:20000/updrp");
    httpReq->isBodyRequired = true;
    httpReq->requestBody = strdup(finalPayload);
    opensubsonic_httpClient_request(&httpReq);
    XNU_HttpRequest(&httpReq);
    
    printf("%s\n", httpReq->responseMsg);
    
    free(finalPayload);
    opensubsonic_httpClient_cleanup(&httpReq);
    
    return 0;
}

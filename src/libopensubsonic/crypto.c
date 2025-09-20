#include <stdio.h>
#include "crypto.h"
#include "../external/md5.h"
#include "../configHandler.h"
#include "logger.h"

#if __NetBSD__ // Functions for NetBSD to use KERN_ARND
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

static int rc = 0;
extern configHandler_config_t* configObj;

// Use arc4random() to generate cryptographically secure bytes. Should work on all BSD-style systems
void crypto_secure_arc4random_generate(unsigned char* bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        bytes[i] = arc4random() & 0xFF;
    }
}

// Use the arandom sysctl on NetBSD to generate cryptographically secure bytes.
#if __NetBSD__
void crypto_secure_netbsd_arandom_generate(unsigned char* bytes, size_t length) {
    // Setup the sysctl MIB for kern.arandom
    int mib[2];
    mib[0] = CTL_KERN;
    mib[1] = KERN_ARND;

    // Read random bytes
    if (sysctl(mib, 2, bytes, &length, NULL, 0) == -1) {
        logger_log_error(__func__, "sysctl() error.");
        exit(EXIT_FAILURE); // TODO handle error better
    }
}
#endif

// Generate an Opensubsonic Login Salt
void crypto_secure_generate_salt(void) {
    uint8_t salt_bytes[8];

    // Generate cryptographically secure salt bytes using OS-native functions
#if __NetBSD__
    crypto_secure_netbsd_arandom_generate(salt_bytes, 8);
#else
    crypto_secure_arc4random_generate(salt_bytes, 8);
#endif

    // Convert to a string hex representation
    char* loginSalt = NULL;
    rc = asprintf(&loginSalt, "%02x%02x%02x%02x%02x%02x%02x%02x",
        salt_bytes[0], salt_bytes[1], salt_bytes[2], salt_bytes[3],
        salt_bytes[4], salt_bytes[5], salt_bytes[6], salt_bytes[7]);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return; // TODO return proper error
    }
    
    // TODO Fix this hack - Copy login salt to config
    configObj->internal_opensubsonic_loginSalt = strdup(loginSalt);
    free(loginSalt);
}

// Generate an MD5 checksum in string hex representation of the account password and salt
void crypto_secure_generate_token(void) {
    uint8_t md5_bytes[16];
    char* token_plaintext = NULL;

    // Concatenate account password and salt into single string
    rc = asprintf(&token_plaintext, "%s%s", configObj->opensubsonic_password, configObj->internal_opensubsonic_loginSalt);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return; // TODO return error
    }
    
    // Generate an MD5 checksum of the plaintext token
    md5String(token_plaintext, md5_bytes);
    free(token_plaintext);

    // Convert the MD5 checksum bytes into string hex representation
    char* loginToken = NULL;
    rc = asprintf(&loginToken, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        md5_bytes[0], md5_bytes[1], md5_bytes[2], md5_bytes[3],
        md5_bytes[4], md5_bytes[5], md5_bytes[6], md5_bytes[7],
        md5_bytes[8], md5_bytes[9], md5_bytes[10], md5_bytes[11],
        md5_bytes[12], md5_bytes[13], md5_bytes[14], md5_bytes[15]);
    if (rc == -1) {
        logger_log_error(__func__, "asprintf() failed.");
        return; // TODO return error
    }
    
    // TODO Fix this hack - Copy login token to config
    configObj->internal_opensubsonic_loginToken = strdup(loginToken);
    free(loginToken);
}

void opensubsonic_crypto_generateLogin(void) {
    crypto_secure_generate_salt();
    crypto_secure_generate_token();
}

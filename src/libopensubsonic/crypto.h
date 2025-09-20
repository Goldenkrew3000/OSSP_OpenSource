#ifndef _CRYPTO_H
#define _CRYPTO_H

// OS-native cryptographic generation functions
void crypto_secure_arc4random_generate(unsigned char* bytes, size_t length);
#if __NetBSD__
void crypto_secure_netbsd_arandom_generate(unsigned char* bytes, size_t length);
#endif

void crypto_secure_generate_salt(void);
void crypto_secure_generate_token(void);
void opensubsonic_crypto_generateLogin(void);

#endif

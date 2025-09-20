/*
 * Libcurl curl_easy_escape out-of-tree and reimplemented
 * Based on libcurl 8.15.0
 * Libcurl Copyright (C) Steve Holme, <steve_holme@hotmail.com>
 * Written for use in OSSP by Goldenkrew3000, <avery@hojuix.org>
 * Acronyms are used to avoid conflict when in use in combination with libcurl (lcue -> Libcurl URI Encode)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcurl_uriescape.h"

#define LCUE_ISUPPER(x)  (((x) >= 'A') && ((x) <= 'Z'))
#define LCUE_ISLOWER(x)  (((x) >= 'a') && ((x) <= 'z'))
#define LCUE_ISDIGIT(x)  (((x) >= '0') && ((x) <= '9'))
#define LCUE_ISALNUM(x)  (LCUE_ISDIGIT(x) || LCUE_ISLOWER(x) || LCUE_ISUPPER(x))
#define LCUE_ISURLPUNTCS(x) (((x) == '-') || ((x) == '.') || ((x) == '_') || ((x) == '~'))
#define LCUE_ISUNRESERVED(x) (LCUE_ISALNUM(x) || LCUE_ISURLPUNTCS(x))
const unsigned char lcue_udigits[] = "0123456789ABCDEF";
const unsigned char lcue_ldigits[] = "0123456789abcdef";

static void lcue_hexbyte(unsigned char* dest, unsigned char val) {
    dest[0] = lcue_udigits[val >> 4];
    dest[1] = lcue_udigits[val & 0x0F];
}

char* lcue_uriescape(char* string, int inlength) {
    size_t length = 0;
    char* d = NULL;
    int d_idx = 0;

    if (!string || (inlength < 0)) {
        return NULL;
    }

    length = (inlength ? (size_t)inlength : strlen(string));
    if(!length) {
        return NULL;
    }

    d = calloc(length * 3 + 1, sizeof(char));
    if (d == NULL) {
        return NULL;
    }

    while (length--) {
        unsigned char in = (unsigned char)*string++;

        if (LCUE_ISUNRESERVED(in)) {
            d[d_idx] = in;
            d_idx++;
        } else {
            unsigned char out[3] = { '%' };
            lcue_hexbyte(&out[1], in);
            d[d_idx] = out[0];
            d[d_idx + 1] = out[1];
            d[d_idx + 2] = out[2];
            d_idx += 3;
        }
    }

    return d;
}

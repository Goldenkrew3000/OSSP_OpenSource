/*
 * Libcurl curl_easy_escape out-of-tree and reimplemented
 * Based on libcurl 8.15.0
 * Libcurl Copyright (C) Steve Holme, <steve_holme@hotmail.com>
 * Written for use in OSSP by Goldenkrew3000, <avery@hojuix.org>
 * Acronyms are used to avoid conflict when in use in combination with libcurl (lsue -> Libcurl URI Encode)
 */

#ifndef _LIBCURL_URIESCAPE_H
#define _LIBCURL_URIESCAPE_H

char* lcue_uriescape(char* string, int inlength);

#endif

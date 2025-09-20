#ifndef _SCROBBLE_LASTFM_H
#define _SCROBBLE_LASTFM_H
#include <stdbool.h>
#include "endpoint_getSong.h"

char* opensubsonic_authenticate_lastFm(void);
void opensubsonic_scrobble_lastFm(bool finalize, opensubsonic_getSong_struct* songStruct);

#endif

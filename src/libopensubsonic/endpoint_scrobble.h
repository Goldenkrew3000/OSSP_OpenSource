#ifndef _ENDPOINT_SCROBBLE_H
#define _ENDPOINT_SCROBBLE_H

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
} opensubsonic_scrobble_struct;

int opensubsonic_scrobble_parse(char* data, opensubsonic_scrobble_struct** scrobbleStruct);
void opensubsonic_scrobble_struct_free(opensubsonic_scrobble_struct** scrobbleStruct);

#endif

#ifndef _ENDPOINT_GETINTERNETRADIOSTATIONS_H
#define _ENDPOINT_GETINTERNETRADIOSTATIONS_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    char* id;
    char* name;
    char* streamUrl;
} opensubsonic_getInternetRadioStations_radioStations_struct;

typedef struct {
    char* status;
    int errorCode;
    char* errorMessage;
    int radioStationCount;
    opensubsonic_getInternetRadioStations_radioStations_struct* radioStations;
} opensubsonic_getInternetRadioStations_struct;

int opensubsonic_getInternetRadioStations_parse(char* data, opensubsonic_getInternetRadioStations_struct** getInternetRadioStationsStruct);
void opensubsonic_getInternetRadioStations_struct_free(opensubsonic_getInternetRadioStations_struct** getInternetRadioStationsStruct);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _ENDPOINT_GETINTERNETRADIOSTATIONS_H

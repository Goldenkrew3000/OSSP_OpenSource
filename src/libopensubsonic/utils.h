#ifndef _UTILS_H
#define _UTILS_H
#include <stdbool.h>
#include "../external/cJSON.h"

void OSS_Psoj(char** dest, cJSON* obj, char* child);
void OSS_Pioj(int* dest, cJSON* obj, char* child);
void OSS_Ploj(long* dest, cJSON* obj, char* child);
void OSS_Pboj(bool* dest, cJSON* obj, char* child);

#endif

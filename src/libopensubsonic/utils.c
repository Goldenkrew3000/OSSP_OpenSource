#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "../external/cJSON.h"
#include "logger.h"
#include "utils.h"

/*
 * ACRONYMS:
 * OSS_Psoj -> Opensubsonic_PullStringOutofJson
 * OSS_Pioj -> Opensubsonic_PullIntOutofJson
 * OSS_Ploj -> Opensubsonic_PullLongOutofJson
 * OSS_Pboj -> Opensubsonic_PullBoolOutofJson
 */

void OSS_Psoj(char** dest, cJSON* obj, char* child) {
    if (obj == NULL) {
        //void* ret_addr = __builtin_return_address(0);
        //Dl_info info;
        //if (dladdr(ret_addr, &info)) {
        //    logger_log_error(info.dli_sname, "Parent object is null.");
        //}
    } else {
        cJSON* childObj = cJSON_GetObjectItem(obj, child);
        
        if (cJSON_IsString(childObj) && childObj->valuestring != NULL) {
            *dest = strdup(childObj->valuestring);
        } else {
            //void* ret_addr = __builtin_return_address(0);
            //Dl_info info;
            //if (dladdr(ret_addr, &info)) {
            //    logger_log_error(info.dli_sname, "Object %s is not a string or string is null.", child);
            //}
        }
    }
}

void OSS_Pioj(int* dest, cJSON* obj, char* child) {
    if (obj == NULL) {
        //void* ret_addr = __builtin_return_address(0);
        //Dl_info info;
        //if (dladdr(ret_addr, &info)) {
        //    logger_log_error(info.dli_sname, "Parent object is null.");
        //}
    } else {
        cJSON* childObj = cJSON_GetObjectItem(obj, child);

        if (cJSON_IsNumber(childObj)) {
            *dest = childObj->valueint;
        } else {
            //void* ret_addr = __builtin_return_address(0);
            //Dl_info info;
            //if (dladdr(ret_addr, &info)) {
            //    logger_log_error(info.dli_sname, "Object %s is not an int.", child);
            //}
        }
    }
}

void OSS_Ploj(long* dest, cJSON* obj, char* child) {
    if (obj == NULL) {
        //void* ret_addr = __builtin_return_address(0);
        //Dl_info info;
        //if (dladdr(ret_addr, &info)) {
        //    logger_log_error(info.dli_sname, "Parent object is null.");
        //}
    } else {
        cJSON* childObj = cJSON_GetObjectItem(obj, child);
        
        if (cJSON_IsNumber(childObj)) {
            *dest = childObj->valueint;
        } else {
            //void* ret_addr = __builtin_return_address(0);
            //Dl_info info;
            //if (dladdr(ret_addr, &info)) {
            //    logger_log_error(info.dli_sname, "Object %s is not a long.", child);
            //}
        }
    }
}

void OSS_Pboj(bool* dest, cJSON* obj, char* child) {
    if (obj == NULL) {
        //void* ret_addr = __builtin_return_address(0);
        //Dl_info info;
        //if (dladdr(ret_addr, &info)) {
        //    logger_log_error(info.dli_sname, "Parent object is null.");
        //}
    } else {
        cJSON* childObj = cJSON_GetObjectItem(obj, child);
        
        if (cJSON_IsBool(childObj)) {
            if (cJSON_IsTrue(childObj)) {
                *dest = true;
            }
        } else {
            //void* ret_addr = __builtin_return_address(0);
            //Dl_info info;
            //if (dladdr(ret_addr, &info)) {
            //    logger_log_error(info.dli_sname, "Object is not a bool.");
            //}
        }
    }
}


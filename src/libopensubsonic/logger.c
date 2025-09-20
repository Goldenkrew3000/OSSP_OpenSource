#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

#define LOGGER_COLOR_BLACK "\033[0;30m"
#define LOGGER_COLOR_RED "\033[0;31m"
#define LOGGER_COLOR_GREEN "\033[0;32m"
#define LOGGER_COLOR_YELLOW "\033[0;33m"
#define LOGGER_COLOR_BLUE "\033[0;34m"
#define LOGGER_COLOR_PURPLE "\033[0;35m"
#define LOGGER_COLOR_CYAN "\033[0;36m"
#define LOGGER_COLOR_WHITE "\033[0;37m"

#define LOGGER_MODE_EVERYTHING 1

void logger_init(int mode) {
    printf("%s +\n", __func__);

    printf("%s HAHAHA %s HAHAHA\n", LOGGER_COLOR_RED, LOGGER_COLOR_WHITE);

    printf("%s -\n", __func__);
}

// Log entry into / exit from function
// Action 1 = Entry, Action 2 = Exit
void logger_log_function(const char* function, int action) {
    if (action == 1) {
        printf("%s%s +%s\n", LOGGER_COLOR_YELLOW, function, LOGGER_COLOR_WHITE);
    } else if (action == 2) {
        printf("%s%s -%s\n", LOGGER_COLOR_YELLOW, function, LOGGER_COLOR_WHITE);
    } else {
        printf("%sERROR: Invalid logger action!%s\n", LOGGER_COLOR_RED, LOGGER_COLOR_WHITE);
    }
}

// Log error
void logger_log_error(const char* function, const char* format, ...) {
    printf("%s(%s) ERROR - ", LOGGER_COLOR_RED, function);
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
    printf("%s\n", LOGGER_COLOR_WHITE);
}

// Log general information
void logger_log_general(const char* function, const char* format, ...) {
    printf("%s(%s) ", LOGGER_COLOR_YELLOW, function);
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
    printf("%s\n", LOGGER_COLOR_WHITE);
}

// Log important information
void logger_log_important(const char* function, const char* format, ...) {
    printf("%s(%s) ", LOGGER_COLOR_GREEN, function);
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
    printf("%s\n", LOGGER_COLOR_WHITE);
}

// Log title
void logger_log_title(const char* function, const char* format, ...) {
    printf("%s(%s) ", LOGGER_COLOR_PURPLE, function);
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
    printf("%s\n", LOGGER_COLOR_WHITE);
}

#ifndef _LOGGER_H
#define _LOGGER_H

void logger_init(int mode);
void logger_log_function(const char* function, int action);
void logger_log_error(const char* function, const char* format, ...);
void logger_log_general(const char* function, const char* format, ...);
void logger_log_important(const char* function, const char* format, ...);
void logger_log_title(const char* function, const char* format, ...);

#endif

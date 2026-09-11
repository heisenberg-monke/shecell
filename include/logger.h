#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

void Logger_setDebug(bool debug);
bool Logger_debugEnabled();

void Logger_log(const char *level, const char *func, const char *fmt, ...);

#define LOG_INFO(...) if(Logger_debugEnabled()) Logger_log("INFO", __func__, __VA_ARGS__)
#define LOG_WARN(...) if(Logger_debugEnabled()) Logger_log("WARN", __func__, __VA_ARGS__)
#define LOG_ERR(...)  Logger_log("ERROR", __func__, __VA_ARGS__)
#define LOG_OUT(...)  Logger_log(NULL, NULL, __VA_ARGS__)

#endif
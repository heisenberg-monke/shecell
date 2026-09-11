#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include <stdio.h>

void Logger_setDebug(bool debug);
bool Logger_debugEnabled();

void Logger_log(FILE *file, const char *level, const char *func, const char *fmt, ...);

#define LOG_INFO(...) if(Logger_debugEnabled()) Logger_log(stdout, "INFO", __func__, __VA_ARGS__)
#define LOG_WARN(...) if(Logger_debugEnabled()) Logger_log(stderr, "WARN", __func__, __VA_ARGS__)
#define LOG_ERR(...)  Logger_log(stderr, "ERROR", __func__, __VA_ARGS__)
#define LOG_OUT(...)  Logger_log(stdout, NULL, NULL, __VA_ARGS__)

#endif
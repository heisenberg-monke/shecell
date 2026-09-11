#include "logger.h"

#include <stdio.h>
#include <stdarg.h>

static bool g_debug = false;

void Logger_setDebug(bool debug) {
    g_debug = debug;
}

bool Logger_debugEnabled() {
    return g_debug;
}

void Logger_log(FILE *file, const char *level, const char *func, const char *fmt, ...)
{
    va_list args;

    if(level)
        fprintf(file, "[%s] ", level);

    if(func)
        fprintf(file, "%s: ", func);

    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
}
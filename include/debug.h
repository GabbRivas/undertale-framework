#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#if defined(DEBUG)

#include <time.h>

extern FILE *debug_log_pointer;

#define debug_print(message, ...)                         \
    do {                                                  \
        if (debug_log_pointer) {                          \
            time_t t = time(NULL);                        \
            struct tm tm_buf;                             \
            localtime_s(&tm_buf, &t);                     \
            fprintf(debug_log_pointer,                   \
                    "[%02d:%02d:%02d] ",                 \
                    tm_buf.tm_hour,                      \
                    tm_buf.tm_min,                       \
                    tm_buf.tm_sec);                      \
            fprintf(debug_log_pointer, message,           \
                    ##__VA_ARGS__);                       \
        }                                                 \
    } while (0)

#else
#define debug_print(message, ...)
#endif

#endif // DEBUG_H

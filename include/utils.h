#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdlib.h>

typedef struct StringView
{
    const char *begin;
    size_t length;
}
StringView;

#define SV_FMT "%.*s"
#define SV_ARG(sv) (int) (sv)->length, (sv)->begin

#define VEC_INIT_CAP 16

#define VEC_RESERVE(vec, cap)                                                               \
    do                                                                                      \
    {                                                                                       \
        if((cap) > (vec)->capacity)                                                         \
        {                                                                                   \
            if((vec)->capacity == 0)                                                        \
                (vec)->capacity = VEC_INIT_CAP;                                             \
            while((cap) > (vec)->capacity)                                                  \
                (vec)->capacity *= 2;                                                       \
            (vec)->data = realloc((vec)->data, (vec)->capacity * sizeof(*(vec)->data));     \
        }                                                                                   \
    } while(0)

#define VEC_PUSH(vec, x)                    \
    do                                      \
    {                                       \
        VEC_RESERVE(vec, (vec)->size + 1);  \
        (vec)->data[(vec)->size++] = (x);   \
    } while(0)

#define VEC_BACK(vec) (vec)->data[(vec)->size-1]

#define ARR_LEN(arr) (sizeof(arr) / sizeof(*arr))

#endif
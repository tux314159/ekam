#ifndef DEBUG_H
#define DEBUG_H

#ifndef NDEBUG

#include <stdio.h>

#define debugpf(...) fprintf(stderr, __VA_ARGS__)
#define dbgidntpf(level, ...) do {                      \
    for (size_t ___ = 0; ___ < level; ___++) {          \
        debugpf(" ");                                  \
    }                                                   \
    debugpf(__VA_ARGS__);                               \
} while (0);

#else

#define debugpf(fstring, ...) ;
#define dbgidntpf(level, fstring, ...) ;

#endif

#endif

#ifndef SAFEALLOC_H
#define SAFEALLOC_H

#include <sys/types.h>

void *malloc_s(size_t size);
void *calloc_s(size_t nmemb, size_t size);
void *realloc_s(void *ptr, size_t size);

#endif

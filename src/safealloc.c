#include "safealloc.h"
#include "util/debug.h"
#include <assert.h>
#include <stdlib.h>

void *
malloc_s(size_t size)
{
	void *m = malloc(size);
	NULLDIE(m);
	return m;
}

void *
calloc_s(size_t nmemb, size_t size)
{
	void *m = calloc(nmemb, size);
	NULLDIE(m);
	return m;
}

void *
realloc_s(void *ptr, size_t size)
{
	void *m = realloc(ptr, size);
	NULLDIE(m);
	return m;
}

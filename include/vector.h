#ifndef INCLUDE_DATASTRUCTS_H
#define INCLUDE_DATASTRUCTS_H

#include "safealloc.h"

#define MAKE_VEC_T(type) \
	struct Vec_##type {  \
		size_t sz;       \
		size_t real_sz;  \
		type *arr;       \
	}

#define VEC_INIT(vec)                              \
	do {                                           \
		vec.arr = calloc_s(1, sizeof(*(vec).arr)); \
		vec.sz = 0;                                \
		vec.real_sz = sizeof(*(vec).arr);          \
	} while (0);

#define VEC_KILL(vec) free(vec.arr);

#define VEC_PUSH(vec, x)                                           \
	do {                                                           \
		(vec).arr[(vec).sz++] = x;                                 \
		if (sizeof(*(vec).arr) * ((vec).sz + 1) > (vec).real_sz) { \
			(vec).arr = realloc_s((vec).arr, (vec).real_sz *= 2);  \
		}                                                          \
	} while (0);

#define VEC_POP(vec) vec.sz--;

MAKE_VEC_T(int);
MAKE_VEC_T(size_t);

#endif

#ifndef INCLUDE_HASHTABLE_H
#define INCLUDE_HASHTABLE_H

#include "safealloc.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

/*
 * This is not a very fast implementation of a hash table:
 * it uses seperate chaining, and isn't really optimised.
 * However I think it works well enough.
 */

unsigned long
ht_hash(const char *str);

unsigned long
ht_hash(const char *str)
{
	// djb2 hash algorithm
	unsigned long hash = 5381;
	char          c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + (unsigned char)c;
	}

	return hash;
}

#define MAKE_HT_T(T)                                                          \
	typedef struct HT_entry_##T {                                             \
		struct HT_entry_##T *next;                                            \
		char                *key;                                             \
		T                    data;                                            \
		struct HT_entry_##T *iter_next;                                       \
	} HT_entry_##T;                                                           \
                                                                              \
	typedef struct Hashtable_##T {                                            \
		/* since this is a hash table it is likely that data will be stored   \
		 * in the first bucket. To optimise for this case we will store all   \
		 * first elements in sequential memory. */                            \
		struct HT_entry_##T *entries;                                         \
		size_t               size;                                            \
		size_t               n_buckets;                                       \
		struct HT_entry_##T *iter_begin;                                      \
		struct HT_entry_##T *iter_end;                                        \
		struct HT_entry_##T  iter_dummy;                                      \
	} Hashtable_##T;                                                          \
                                                                              \
	Hashtable_##T *ht_create_##T(size_t size);                                \
	void           ht_destroy_##T(Hashtable_##T *ht);                         \
	void           ht_insert_##T(Hashtable_##T *ht, const char *key, T data); \
	T             *ht_get_##T(Hashtable_##T *ht, const char *key);            \
                                                                              \
	Hashtable_##T *ht_create_##T(size_t size)                                 \
	{                                                                         \
		Hashtable_##T *ht = malloc_(sizeof(*ht));                             \
		ht->entries       = calloc_s(size, sizeof(*(ht->entries)));           \
		ht->size          = 0;                                                \
		ht->n_buckets     = size;                                             \
		ht->iter_begin    = NULL;                                             \
		ht->iter_end      = NULL;                                             \
		ht->iter_dummy =                                                      \
			(HT_entry_##T){.next = NULL, .key = NULL, .iter_next = NULL};     \
		return ht;                                                            \
	}                                                                         \
                                                                              \
	void ht_destroy_##T(Hashtable_##T *ht)                                    \
	{                                                                         \
		for (HT_entry_##T *p = ht->entries; p < ht->entries + ht->n_buckets;  \
		     p++) {                                                           \
			if (!p->next) {                                                   \
				continue;                                                     \
			}                                                                 \
			HT_entry_##T *pp   = p->next;                                     \
			HT_entry_##T *prev = pp;                                          \
			while ((pp = pp->next)) {                                         \
				free(prev);                                                   \
				prev = pp;                                                    \
			}                                                                 \
		}                                                                     \
		free(ht->entries);                                                    \
	}                                                                         \
                                                                              \
	void ht_insert_##T(Hashtable_##T *ht, const char *key, T data)            \
	{                                                                         \
		unsigned long hash = ht_hash(key) % ht->n_buckets;                    \
		HT_entry_##T *nent = calloc_s(1, sizeof(*nent));                      \
		HT_entry_##T *p    = ht->entries + hash;                              \
		while (p->next) {                                                     \
			p = p->next;                                                      \
		}                                                                     \
		p->next = nent;                                                       \
		p->key  = malloc_(sizeof(*key) * (strlen(key) + 1));                  \
		strcpy(p->key, key);                                                  \
		p->data      = data;                                                  \
		p->iter_next = &ht->iter_dummy;                                       \
		if (ht->iter_begin) {                                                 \
			ht->iter_end->iter_next = p;                                      \
		} else {                                                              \
			ht->iter_begin = p;                                               \
		}                                                                     \
		ht->iter_end = p;                                                     \
		ht->size++;                                                           \
	}                                                                         \
                                                                              \
	T *ht_get_##T(Hashtable_##T *ht, const char *key)                         \
	{                                                                         \
		unsigned long hash = ht_hash(key) % ht->n_buckets;                    \
		HT_entry_##T *p    = ht->entries + hash;                              \
		while (p->next) {                                                     \
			if (!strcmp(p->key, key)) {                                       \
				return &p->data;                                              \
			}                                                                 \
			p = p->next;                                                      \
		}                                                                     \
		return NULL;                                                          \
	}

#define HT_ITER(ht, pname, T) \
	for (HT_entry_##T *p = ht->iter_begin; p->iter_next; p = p->iter_next)

MAKE_HT_T(int)
MAKE_HT_T(size_t)

#endif

#ifndef INCLUDE_HASHTABLE_H
#define INCLUDE_HASHTABLE_H

#include "safealloc.h"
#include <stdlib.h>
#include <string.h>

/*
 * This is not a very fast implementation of a hash table:
 * it uses seperate chaining, and isn't really optimised.
 * However I think it works well enough.
 */

unsigned long ht_hash(const char *str);

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
	} HT_entry_##T;                                                           \
                                                                              \
	typedef struct Hashtable_##T {                                            \
		struct HT_entry_##T *entries;                                         \
		size_t               size;                                            \
		size_t               n_buckets;                                       \
	} Hashtable_##T;                                                          \
                                                                              \
	Hashtable_##T *ht_create_##T(size_t size);                                \
	void           ht_destroy_##T(Hashtable_##T *ht);                         \
	void           ht_insert_##T(Hashtable_##T *ht, const char *key, T data); \
	T             *ht_get_##T(Hashtable_##T *ht, const char *key);            \
                                                                              \
	Hashtable_##T *ht_create_##T(size_t size)                                 \
	{                                                                         \
		Hashtable_##T *ht = malloc(sizeof(*ht));                              \
		ht->entries       = calloc_s(size, sizeof(*(ht->entries)));           \
		ht->size          = 0;                                                \
		ht->n_buckets     = size;                                             \
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
		HT_entry_##T *nent = calloc(1, sizeof(*nent));                        \
		HT_entry_##T *p    = ht->entries + hash;                              \
		while (p->next) {                                                     \
			p = p->next;                                                      \
		}                                                                     \
		p->next = nent;                                                       \
		p->key  = malloc(sizeof(*key) * strlen(key));                         \
		strcpy(p->key, key);                                                  \
		p->data = data;                                                       \
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

#endif

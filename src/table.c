#include "table.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t
djb2(const unsigned char *str)
{
	uint32_t hash = 5381;
	unsigned char c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

int
table_init(struct Table *tbl)
{
	tbl->n_slots = TABLE_INIT_SLOTS;
	tbl->n_filled = 0;
	tbl->n_tomb = 0;
	tbl->slots = calloc(2 * TABLE_INIT_SLOTS, sizeof(*tbl->slots));
	if (!tbl->slots)
		goto cleanup_fail;
	return 0;

cleanup_fail:
	return -1;
}

void
table_destroy(struct Table *tbl)
{
	for (size_t i = 0; i < tbl->n_slots; i++) {
		if (!tbl->slots[i].key || tbl->slots[i].key == (char *)&tbl->n_tomb)
			continue;
		free(tbl->slots[i].key);
	}
}

/* Just double the capacity of the table. */
static int
_table_resize(struct Table *tbl)
{
	uint32_t slot;
	const size_t old_cap = tbl->n_slots;
	struct TableEntry *new_slots;

	tbl->n_slots <<= 1;
	new_slots = calloc(2 * tbl->n_slots, sizeof(*new_slots));
	if (!new_slots)
		goto cleanup_fail;

	for (size_t i = 0; i < old_cap; i++) {
		if (!tbl->slots[i].key || tbl->slots[i].key == (char *)&tbl->n_tomb)
			continue;
		/* Find new slot */
		slot = djb2((unsigned char *)tbl->slots[i].key) &
		       (tbl->n_slots - 1); /* hash % n */
		while (new_slots[slot].key)
			slot = (slot + 1) & (tbl->n_slots - 1);
		new_slots[slot].key = tbl->slots[i].key;
		new_slots[slot].val = tbl->slots[i].val;
	}
	free(tbl->slots);
	tbl->slots = new_slots;
	tbl->n_tomb = 0;

	return 0;
cleanup_fail:
	return -1;
}

/* We don't want to keep reallocating memory even when the required capacity
 * never changes. */
static void
_table_exhume(struct Table *tbl)
{
	struct TableEntry *tmp;
	size_t slot;

	memset(tbl->more, 0, tbl->n_slots * sizeof(*tbl->more));

	for (size_t i = 0; i < tbl->n_slots; i++) {
		if (!tbl->slots[i].key || tbl->slots[i].key == (char *)&tbl->n_tomb)
			continue;
		/* Find new slot */
		slot = djb2((unsigned char *)tbl->slots[i].key) &
		       (tbl->n_slots - 1); /* hash % n */
		while (tbl->more[slot].key)
			slot = (slot + 1) & (tbl->n_slots - 1);
		tbl->more[slot].key = tbl->slots[i].key;
		tbl->more[slot].val = tbl->slots[i].val;
	}
	tbl->n_tomb = 0;

	tmp = tbl->more;
	tbl->more = tbl->slots;
	tbl->slots = tmp;
}

static inline struct TableEntry *
_table_find(struct Table *tbl, const char *key)
{
	uint32_t slot;

	slot = djb2((const unsigned char *)key) & (tbl->n_slots - 1);
	while (tbl->slots[slot].key &&
	       (tbl->slots[slot].key == (char *)&tbl->n_tomb ||
	        strcmp(tbl->slots[slot].key, key))) {
		slot = (slot + 1) & (tbl->n_slots - 1);
	}
	if (tbl->slots[slot].key)
		return tbl->slots + slot;
	else
		return NULL;
}

int
table_insert(struct Table *tbl, const char *key, void *val)
{
	uint32_t slot;
	struct TableEntry *ent;

	/* If key already exists then update, else insert */
	ent = _table_find(tbl, key);
	if (ent) {
		ent->val = val;
	} else {
		if (100 * tbl->n_filled / tbl->n_slots > TABLE_RESIZE_RATIO)
			if (_table_resize(tbl))
				goto cleanup_fail;
		if (100 * (tbl->n_filled + tbl->n_tomb) / tbl->n_slots >
		    TABLE_RESIZE_RATIO)
			_table_exhume(tbl);

		slot = djb2((const unsigned char *)key) &
		       (tbl->n_slots - 1); /* hash % n */
		while (tbl->slots[slot].key &&
		       tbl->slots[slot].key != (char *)&tbl->n_tomb)
			slot = (slot + 1) & (tbl->n_slots - 1);

		if (tbl->slots[slot].key == (char *)&tbl->n_tomb)
			tbl->n_tomb--;
		tbl->slots[slot].key = malloc(strlen(key) + 1);
		tbl->slots[slot].val = val;
		if (!tbl->slots[slot].key) {
			free(tbl->slots[slot].key);
			goto cleanup_fail;
		}
		strcpy(tbl->slots[slot].key, key);

		tbl->n_filled++;
	}

	return 0;

cleanup_fail:
	return -1;
}

int
table_delete(struct Table *tbl, const char *key)
{
	struct TableEntry *addr;

	addr = _table_find(tbl, key);
	if (!addr)
		return -1;
	free(addr->key);
	addr->key = (char *)&tbl->n_tomb;
	tbl->n_filled--;
	tbl->n_tomb++;

	return 0;
}

void **
table_find(struct Table *tbl, const char *key)
{
	struct TableEntry *addr;

	addr = _table_find(tbl, key);
	if (!addr)
		return NULL;
	else
		return &addr->val;
}

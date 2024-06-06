#ifndef INCLUDE_TABLE_H
#define INCLUDE_TABLE_H

#include <stddef.h>

/*
 * Quick and dirty hash-tables, using linear probing. We'll just use djb2...
 * There are some tunables in the macros below.
 */

#define TABLE_INIT_SLOTS 32
#define TABLE_RESIZE_RATIO 70

struct TableEntry {
	char *key;
	void *val;
};

struct Table {
	size_t n_slots; /* 2^k */
	size_t n_filled;
	size_t n_tomb; /* address of this field is tombstone */
	struct TableEntry *slots;
	struct TableEntry *more; /* unused half to avoid expensive copy */
};

/* Initialise a table. */
int
table_init(struct Table *tbl);

/* Deallocate a table. */
void
table_destroy(struct Table *tbl);

/* Find a value in a table, return a pointer to a pointer to it, and a NULL
 * pointer if it does not exist.
 * WARNING: If not NULL dereference this pointer immediately as it may be
 * shifted upon subsequent inserts! */
void **
table_find(struct Table *tbl, const char *key);

/* Insert item into table. Key must be null-terminated and can be ephermal,
 * val is simply a pointer and will not be copied. */
int
table_insert(struct Table *tbl, const char *key, void *val);

/* Remove item from table. */
int
table_delete(struct Table *tbl, const char *key);

#endif

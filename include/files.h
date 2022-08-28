#ifndef INCLUDE_FS
#include <sys/queue.h>
#include <ndbm.h>

struct filehash {
   char *filename;
   char hash[41];
   LIST_ENTRY(filehash) entries;
};
LIST_HEAD(listhead, filehash);

// TODO: handle errors

/*
 * Computes the hash of the contents of a file; null-terminated
 * SHA-1 hash is copied into buf.
 */
void
file_compute_sha1(const char *filename, char buf[41]);
/*
 * Walks the filesystem starting from dirname, computes the hash
 * of each regular file with file_compute_sha1(), and stores it
 * in the linked list specified by list.
 */
void
walkdir_storehash(const char *dirname, struct listhead *list);
/*
 * Walks the linked list list and stores each entry in the
 * DBM database db.
 */
void
fhlist_to_db(struct listhead *list, DBM *db);
/*
 * Compares the entries in a list cur to older entries in the
 * DBM database old, and allocates an array holding all the
 * new/changed filenames. WARNING: remember to free the array!
 */
char **
get_changed(struct listhead *cur, DBM *old);
/*
 * Gets the node number of a particular name by consulting the
 * global hashmap.
 */
ssize_t
get_nn(char *name);

#endif

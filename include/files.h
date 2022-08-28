#ifndef INCLUDE_FS
#include <sys/queue.h>
#include <ndbm.h>

struct filehash {
   char *filename;
   char hash[41];
   LIST_ENTRY(filehash) entries;
};
LIST_HEAD(listhead, filehash);

void
file_compute_sha1(const char *filename, char buf[41]);
void
walkdir_storehash(const char *dirname, struct listhead *list);
void
fhlist_to_db(struct listhead *list, DBM *db);
char **
get_changed(struct listhead *cur, DBM *old);
ssize_t
get_nn(char *name);

#endif

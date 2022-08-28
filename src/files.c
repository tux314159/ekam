#include <dirent.h>
#include <fcntl.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sha1.h>

#include "files.h"
#include "safealloc.h"

void
file_compute_sha1(const char *filename, char buf[41])
{
    FILE *fp = fopen(filename, "rb");
    fseek(fp, 0, SEEK_END); 
    size_t fsz = (size_t) ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *fcontent = malloc_s(fsz + 1);
    fread(fcontent, 1, fsz, fp);
	fcontent[fsz] = '\0';

    SHA1Data((uint8_t *)fcontent, strlen(fcontent), buf);
    free(fcontent);
    return;
}

void
walkdir_storehash(const char *dirname, struct listhead *list)
{
    DIR *dir = opendir(dirname);
    struct dirent *nent;
    if (!dir) {
        return;
    }

    while ((nent = readdir(dir))) {
        const char *fname = nent->d_name;
        if (!strcmp(fname, "..") || !strcmp(fname, ".")) {
            continue;
        }

        // Get full pathname
        char *fullpath = malloc_s(strlen(dirname) + strlen(fname) + 2);
        strcpy(fullpath, dirname);
        fullpath[strlen(dirname)] = '/';
        strcpy(fullpath + strlen(dirname) + 1, fname);

        struct stat statbuf;
        stat(fullpath, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {
            walkdir_storehash(fullpath, list);
        } else {
            // Read file, calculate checksum
            char hash[41];
			hash[40] = '\0';
            file_compute_sha1(fullpath, hash);

            // Store it in list
            struct filehash *e = malloc_s(sizeof(struct filehash));
            e->filename = fullpath;
            strcpy(e->hash, hash);
            LIST_INSERT_HEAD(list, e, entries);
        }
    }

    return;
}

void
fhlist_to_db(struct listhead *list, DBM *db)
{
    struct filehash *fh;
    LIST_FOREACH(fh, list, entries) {
        datum key = {
            .dptr = fh->filename,
            .dsize = strlen(fh->filename) + 1
        };
        datum entry = {
            .dptr = fh->hash,
            .dsize = strlen(fh->hash) + 1
        };
        dbm_store(db, key, entry, DBM_REPLACE);
    }

    return;
}

// WARNING: array passed back must be free'd
char **
get_changed(struct listhead *cur, DBM *old)
{
    size_t len = 0;
    size_t size = 1;
    char **arr = malloc_s(sizeof(char *));
    struct filehash *fh;
    // Loop through all current file/hash pairs, check hashes
    LIST_FOREACH(fh, cur, entries) {
        datum key = {
            .dptr = fh->filename,
            .dsize = strlen(fh->filename) + 1
        };
        datum resp = dbm_fetch(old, key);

        if (!resp.dptr || strcmp(resp.dptr, fh->hash)) {
            // New/updated file
            if (len >= size) {
                size *= 2;
                char **tmp = realloc_s(arr, size * sizeof(char *));
                arr = tmp;
            }
            arr[len++] = fh->filename;
        }
    }
    // Null terminator :p
    arr[len] = NULL;

    return arr;
}

ssize_t
get_nn(char *name)
{
	ENTRY e;
	e.key = name;
	ENTRY *r = hsearch(e, FIND);
	if (!r) {
		return -1;
	}
	return (ssize_t)r->data;
}

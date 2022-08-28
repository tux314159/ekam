#include <fcntl.h>
#include <search.h>
#include <stddef.h>
#include <stdlib.h>

#include "files.h"
#include "graph.h"

char dbname[] = ".ekamdb";
const size_t max_targs = 1000;
size_t n = 1;

int
main(int argc, char **argv)
{
	hcreate(max_targs);
	struct Graph _EKAM_MAIN_GRAPH  = graph_make();
	struct Graph _EKAM_PART_GRAPH = graph_make();

	ADD_INITIAL("main.c", n++, "");
	ADD_INITIAL("src/fs.c", n++, "");
	ADD_INITIAL("src/graph.c", n++, "");
	ADD_INITIAL("src/safealloc.c", n++, "");
	ADD_TARGET("build/files.o", n++, "gcc -c -o build/files.o src/files.c", get_nn("src/files.c"));
	ADD_TARGET("build/graph.o", n++, "gcc -c -o build/graph.o src/graph.c", get_nn("src/graph.c"));
	ADD_TARGET("build/safealloc.o", n++, "gcc -c -o build/safealloc.o src/safealloc.c", get_nn("src/safealloc.c"));
	ADD_TARGET("build/main.o", n++, "gcc -c -o build/main.o main.c", get_nn("main.c"));
	ADD_TARGET("main", n++, "gcc -c -o build/main.o main.c -lpthread -lrt -lmd -lgdbm -lgdbm_compat", get_nn("build/main.o"), get_nn("build/files.o"), get_nn("build/graph.o"), get_nn("build/safealloc.o"));

    DBM *db = dbm_open(dbname, O_CREAT | O_RDWR, 0644);
    struct listhead list;
    LIST_INIT(&list);
    walkdir_storehash(".", &list);
	char **changed = get_changed(&list, db);
	char **s = changed;
	while (*s) {
		printf("%s updated\n", *s++);
	}

	BUILDPARTIAL(2);
	graph_execute(&_EKAM_PART_GRAPH, argc == 1 ? 1 : atoi(argv[1]));
	graph_delete(&_EKAM_PART_GRAPH);
	graph_delete(&_EKAM_MAIN_GRAPH);

	fhlist_to_db(&list, db);

	return 0;
}

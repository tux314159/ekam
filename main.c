#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>

#include "fs.h"
#include "graph.h"

int
main(int argc, char **argv)
{
	struct Graph g  = graph_make();
	struct Graph pg = graph_make();
    ADD_TARGET(&g, 10, "echo at 10; sleep 1.1", 1, 2);
    ADD_TARGET(&g, 1, "echo at 1; sleep 0.3", 3, 4);
    ADD_TARGET(&g, 2, "echo at 2; sleep 0.9", 5);
    ADD_INITIAL(&g, 3, "echo at 3; sleep 1.0");
    ADD_TARGET(&g, 4, "echo at 4; sleep 1.1", 5);
    ADD_INITIAL(&g, 5, "echo at 5; sleep 0.1");
    ADD_TARGET(&g, 6, "echo at 6; sleep 0.3", 2, 8, 9);
    ADD_TARGET(&g, 7, "echo at 7; sleep 0.8", 1, 10, 6);
    ADD_TARGET(&g, 8, "echo at 8; sleep 2.0", 5);
    ADD_TARGET(&g, 9, "echo at 9; sleep 0.2", 5);

	graph_buildpartial(&g, &pg, (size_t[]){5}, 1);
	graph_execute(&pg, argc == 1 ? 1 : atoi(argv[1]));
	graph_delete(&pg);
	graph_delete(&g);

    //DBM *db = dbm_open(dbname, O_CREAT | O_RDWR, 0644);
    struct listhead list;
    LIST_INIT(&list);
    walkdir_storehash(".", &list);
	return 0;
}

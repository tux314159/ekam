#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "hashtable.h"

MAKE_HT_T(int)

int
main(int argc, char **argv)
{
	struct Graph g  = graph_make();
	struct Graph pg = graph_make();
	ADD_TARGET(&g, 10, "echo at 10; sleep 1.0; touch test/10", "test/10", 1, 2);
	ADD_TARGET(&g, 1, "echo at 1; sleep 1.0; touch test/1", "test/1", 3, 4);
	ADD_TARGET(&g, 2, "echo at 2; sleep 1.0; touch test/2", "test/2", 5);
	ADD_INITIAL(&g, 3, "echo at 3; sleep 1.0; touch test/3", "test/3");
	ADD_TARGET(&g, 4, "echo at 4; sleep 1.0; touch test/4", "test/4", 5);
	ADD_INITIAL(&g, 5, "echo at 5; sleep 1.0; touch test/5", "test/5");
	ADD_TARGET(&g, 6, "echo at 6; sleep 1.0; touch test/6", "test/6", 2, 8, 9);
	ADD_TARGET(&g, 7, "echo at 7; sleep 1.0; touch test/7", "test/7", 1, 10, 6);
	ADD_TARGET(&g, 8, "echo at 8; sleep 1.0; touch test/8", "test/8", 3);
	ADD_TARGET(&g, 9, "echo at 9; sleep 1.0; touch test/9", "test/9", 3);

	/* Hashtable test because yes. */
	Hashtable_int *ht = ht_create_int(2);
	ht_insert_int(ht, "hello", 3);
	ht_insert_int(ht, "ohey", 1);
	ht_insert_int(ht, "miao", 4);
	printf(
		"%d %d %d %p\n",
		*ht_get_int(ht, "hello"),
		*ht_get_int(ht, "ohey"),
		*ht_get_int(ht, "miao"),
		(void*)ht_get_int(ht, "boo")
	);
	ht_destroy_int(ht);

	graph_buildpartial(&g, &pg, 7);
	graph_execute(&pg, argc == 1 ? 1 : atoi(argv[1]));
	graph_delete(&pg);
	graph_delete(&g);
	return 0;
}

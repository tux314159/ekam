#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "hashtable.h"
#include "ekam.h"

int
main(int argc, char **argv)
{
	INIT_EKAM();

	ADD_TARGET("echo at 10; sleep 1.0; touch test/10", "test/10", 2, 3);
	ADD_TARGET("echo at 1; sleep 1.0; touch test/1", "test/1", 4, 5);
	ADD_TARGET("echo at 2; sleep 1.0; touch test/2", "test/2", 6);
	ADD_INITIAL("echo at 3; sleep 1.0; touch test/3", "test/3");
	ADD_TARGET("echo at 4; sleep 1.0; touch test/4", "test/4", 6);
	ADD_INITIAL("echo at 5; sleep 1.0; touch test/5", "test/5");
	ADD_TARGET("echo at 6; sleep 1.0; touch test/6", "test/6", 3, 9, 10);
	ADD_TARGET("echo at 7; sleep 1.0; touch test/7", "test/7", 2, 1, 7);
	ADD_TARGET("echo at 8; sleep 1.0; touch test/8", "test/8", 4);
	ADD_TARGET("echo at 9; sleep 1.0; touch test/9", "test/9", 4);

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
	printf("Iterating hashtable: ");
	HT_ITER(ht, p, int) {
		printf("%s: %d; ", p->key, p->data);
	}
	printf("\n");
	ht_destroy_int(ht);

	BUILD_TARGET(7);
	FREE_EKAM();
}

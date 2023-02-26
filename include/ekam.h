#ifndef INCLUDE_EKAM
#define INCLUDE_EKAM

#include "graph.h"
#include "hashtable.h"

// All the macros for our DSL.

// GENERAL-PURPOSE MACROS

// INIT STUFF
#define INIT_EKAM()                                                       \
	struct Graph      _main_graph     = graph_make();                     \
	struct Graph      _main_partgraph = graph_make();                     \
	Hashtable_size_t *_main_ht        = ht_create_size_t(ADJLIST_MAP_SZ); \
	size_t            _main_cnt       = 1;

// GRAPH CONVENIENCE MACROS
#define ADD_TARGET(cmd, filename, ...)                                   \
	do {                                                                 \
		graph_add_target(                                                \
			&_main_graph,                                                \
			_main_cnt++,                                                 \
			(size_t[]){__VA_ARGS__},                                                     \
			sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t),            \
			cmd,                                                         \
			filename                                                     \
		);                                                               \
		ht_insert_size_t(_main_ht, filename, _main_cnt);                 \
	} while (0)

#define ADD_INITIAL(cmd, filename)                                           \
	do {                                                                     \
		graph_add_target(&_main_graph, _main_cnt++, NULL, 0, cmd, filename); \
	} while (0)

#define BUILD_TARGET(targ)                                              \
	do {                                                                \
		graph_buildpartial(&_main_graph, &_main_partgraph, targ);       \
		graph_execute(&_main_partgraph, argc == 1 ? 1 : atoi(argv[1])); \
	} while (0)

#define FREE_EKAM()                     \
	do {                                \
		graph_delete(&_main_partgraph); \
		graph_delete(&_main_graph);     \
		ht_destroy_size_t(_main_ht);    \
	} while (0)

#endif

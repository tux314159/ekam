#ifndef INCLUDE_EKAM
#define INCLUDE_EKAM

#include "graph.h"
#include "hashtable.h"

// All the macros for our DSL.

// GENERAL-PURPOSE MACROS

#define Q(x) #x

// INIT STUFF
#define INIT_EKAM()                                                       \
	struct Graph      _main_graph     = graph_make();                     \
	struct Graph      _main_partgraph = graph_make();                     \
	Hashtable_size_t *_main_ht        = ht_create_size_t(ADJLIST_MAP_SZ); \
	size_t            _main_cnt       = 1;

// GRAPH CONVENIENCE MACROS

// Resolve a target name.
#define R(x) *ht_get_size_t(_main_ht, Q(x))

// We require all targets to be forward-declared.
#define DECLARE(target) ht_insert_size_t(_main_ht, Q(target), _main_cnt++);

// Define a target.
#define _(filename, cmd, ...)                                 \
	do {                                                      \
		graph_add_target(                                     \
			&_main_graph,                                     \
			R(filename),                                      \
			(size_t[]){__VA_ARGS__},                          \
			sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t), \
			Q(cmd),                                           \
			Q(filename)                                       \
		);                                                    \
	} while (0)

// Ugly; define a target with no dependencies.
#define __(filename, cmd) \
	do {                  \
		graph_add_target( \
			&_main_graph, \
			R(filename),  \
			NULL,         \
			0,            \
			Q(cmd),       \
			Q(filename)   \
		);                \
	} while (0)

#define BUILD_TARGET(targ)                                              \
	do {                                                                \
		graph_buildpartial(&_main_graph, &_main_partgraph, R(targ));    \
		graph_execute(&_main_partgraph, argc == 1 ? 1 : atoi(argv[1])); \
	} while (0)

#define FREE_EKAM()                     \
	do {                                \
		graph_delete(&_main_partgraph); \
		graph_delete(&_main_graph);     \
		ht_destroy_size_t(_main_ht);    \
	} while (0)

#endif

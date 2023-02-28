#ifndef INCLUDE_EKAM
#define INCLUDE_EKAM

#include "build.h"
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

#define FREE_EKAM()                     \
	do {                                \
		graph_delete(&_main_partgraph); \
		graph_delete(&_main_graph);     \
		ht_destroy_size_t(_main_ht);    \
	} while (0)

// GRAPH CONVENIENCE MACROS

// Most macros here that deal with target names will have two
// versions: quoted and unquoted. Quoted versions simply quote
// all applicable arguments and pass them to the unquoted version.

// (R)esolve a target name.
#define R_(x) *ht_get_size_t(_main_ht, x)
#define R(x)  R_(Q(x))

// We require all targets to be forward-declared.
#define DECLARE_(target) ht_insert_size_t(_main_ht, target, _main_cnt++);
#define DECLARE(target)  DECLARE_(Q(target))

// (D)ependency declaration
#define D_(filename, cmd, ...)                                \
	do {                                                      \
		graph_add_target(                                     \
			&_main_graph,                                     \
			R_(filename),                                     \
			(size_t[]){__VA_ARGS__},                          \
			sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t), \
			cmd,                                              \
			filename                                          \
		);                                                    \
	} while (0)
#define D(filename, cmd, ...) D_(Q(filename), Q(cmd), __VA_ARGS__)

// (D)ependency declaration but no actual deps
#define D0_(filename, cmd)                                                    \
	do {                                                                      \
		graph_add_target(&_main_graph, R_(filename), NULL, 0, cmd, filename); \
	} while (0)
#define D0(filename, cmd) D0_(Q(filename), Q(cmd))

#define BUILD_TARGET_(targ)                                           \
	do {                                                              \
		cons_partgraph(&_main_graph, &_main_partgraph, R_(targ));     \
		build_graph(&_main_partgraph, argc == 1 ? 1 : atoi(argv[1])); \
	} while (0)
#define BUILD_TARGET(targ) BUILD_TARGET_(Q(targ))

#endif

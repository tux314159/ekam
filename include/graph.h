#ifndef INCLUDE_GRAPH
#define INCLUDE_GRAPH

#include <sys/types.h>

#define MAX_NODES 1024

struct Node {
	size_t *adj;
	size_t  len;
	char   *cmd;
};

// WARNING: 0 is reserved
struct Graph {
	struct Node *graph;
	struct Node *rgraph;
	size_t       n_nodes;
};

/*
 * Create an empty graph.
 */
struct Graph
graph_make(void);
/*
 * Delete a graph created by graph_create().
 */
void
graph_delete(struct Graph *g);
/*
 * Add a node to the adjacency list;
 * checks for duplicates.
 * TODO: optimise
 */
void
adjlist_add(struct Node *from, size_t to);
/*
 * Add an edge between two nodes.
 * It's "bidirectional": adds one direction to graph,
 * the other direction to rgraph.
 */
void
graph_add_edge(struct Graph *g, size_t from, size_t to);
/*
 * Add a rule to a target. The rule will is a shell command that
 * will be executed.
 */
void
graph_add_rule(struct Graph *g, size_t at, const char *cmd);
/*
 * Add a target, along with dependencies. This is mostly a
 * wrapper around the simple functions described above. Also
 * maps filename <-> node number for later use in the hashmap.
 */
void
graph_add_target(
	struct Graph *g,
	const char   *filename,
	size_t        target,
	size_t       *deps,
	size_t        n_deps,
	const char   *cmd
);
/*
 * Useless piece of shit that's here for some reason,
 * constructs a so-called "partial graph" based on what
 * has been updated. Also creates the node 0, which is the
 * starting point for graph_execute().
 */
void
graph_buildpartial(
	struct Graph *src,
	struct Graph *dest,
	size_t       *starts,
	size_t        n_starts
);
/*
 * Execute dependency graph, possibly in order.
 */
void
graph_execute(struct Graph *g, int max_childs);

// Macros for convenience (use them!)
#define ADD_TARGET(filename, targ, cmd, ...)              \
	graph_add_target(                                     \
		&_EKAM_MAIN_GRAPH,                                \
		filename,                                         \
		targ,                                             \
		(size_t[]){__VA_ARGS__},                          \
		sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t), \
		cmd                                               \
	)
#define ADD_INITIAL(filename, targ, cmd) \
	graph_add_target(&_EKAM_MAIN_GRAPH, filename, targ, NULL, 0, cmd);
#define BUILDPARTIAL(...)                                \
	graph_buildpartial(                                  \
		&_EKAM_MAIN_GRAPH,                               \
		&_EKAM_PART_GRAPH,                               \
		(size_t[]){__VA_ARGS__},                         \
		sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t) \
	)

#endif

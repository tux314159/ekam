#ifndef INCLUDE_GRAPH
#define INCLUDE_GRAPH

#include <stdbool.h>
#include <sys/types.h>

#define MAX_NODES 1024

struct Node {
	size_t *adj;
	size_t  len;
    bool    exists;
	char   *cmd;
	char   *filename; // TODO generalise
};

// WARNING: 0 is reserved
struct Graph {
	struct Node *nodes;
	size_t       n_nodes;
};

/*
 * Create a graph.
 */
struct Graph
graph_make(void);

/*
 * Delete a graph.
 */
void
graph_delete(struct Graph *g);

/*
 * Add a unidirectional edge between two nodes, creating them if necessary
 */
void
graph_add_edge(struct Graph *g, size_t from, size_t to);

/*
 * Add metadata to a node.
 * TODO generalise
 */
void
graph_add_meta(
	struct Graph *g,
	size_t        at,
	const char   *cmd,
	const char   *filename
);

/*
 * Wrapper around graph_add_meta and graph_add_edge
 */
void
graph_add_target(
	struct Graph *g,
	size_t        target,
	size_t       *deps,
	size_t        n_deps,
	const char   *cmd,
	const char   *filename
);

/*
 * Build the partial graph containg all nodes in need of updating
 * starting from some starting points. Prunes nodes based on update
 * time comparisons.
 */
void
graph_buildpartial(
	struct Graph *src,
	struct Graph *dest,
	size_t        start
);

/*
 * Execute a graph, possibly in parallel.
 */
void
graph_execute(struct Graph *g, int max_childs);

/*
 * Some convenient macros
 */

#define ADD_TARGET(g, targ, cmd, filename, ...)           \
	graph_add_target(                                     \
		g,                                                \
		targ,                                             \
		(size_t[]){__VA_ARGS__},                          \
		sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t), \
		cmd,                                              \
        filename                                          \
	);

#define ADD_INITIAL(g, targ, cmd, filename) graph_add_target(g, targ, NULL, 0, cmd, filename);

#endif

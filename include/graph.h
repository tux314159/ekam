#ifndef INCLUDE_GRAPH
#define INCLUDE_GRAPH

#include <stdbool.h>
#include <sys/types.h>

#include "vector.h"

// absolute maximum number of nodes
#define MAX_NODES 100000
// max size of map in each adjlist; larger sizes will
// make it faster but take more memory
#define ADJLIST_MAP_SZ 65536

struct Node {
	size_t *adj;
	size_t len;
	bool exists;
	char *cmd;
	char *filename; // TODO generalise
};

// WARNING: 0 is reserved
struct Graph {
	struct Node *nodes;
	size_t n_nodes;
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
 * Add a unidirectional edge between two nodes, creating them if necessary.
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
	size_t at,
	const char *cmd,
	const char *filename
);

/*
 * Wrapper around graph_add_meta and graph_add_edge.
 */
void
graph_add_target(
	struct Graph *g,
	size_t target,
	size_t *deps,
	size_t n_deps,
	const char *cmd,
	const char *filename
);

/*
 * Copy a graph src into dest, possibly inverting the edges.
 */
void
graph_copy(struct Graph *src, struct Graph *dest, size_t start, bool invert);

/*
 * Topologically sort a graph.
 */
void
toposort(struct Graph *g, size_t c, struct Vec_size_t *tsorted, char *visited);

// We're doing BFS everywhere - why not macro?
#define BFS_BEGIN(                                                \
	_BFS_BEGIN_Q,                                                 \
	_BFS_BEGIN_QSZ,                                               \
	_BFS_BEGIN_H,                                                 \
	_BFS_BEGIN_T,                                                 \
	_BFS_BEGIN_VIS,                                               \
	_BFS_BEGIN_START                                              \
)                                                                 \
	size_t _BFS_BEGIN_Q[_BFS_BEGIN_QSZ];                          \
	size_t _BFS_BEGIN_H = 0, _BFS_BEGIN_T = 1;                    \
	_BFS_BEGIN_Q[0] = _BFS_BEGIN_START;                           \
	char _BFS_BEGIN_VIS[_BFS_BEGIN_QSZ];                          \
	memset(_BFS_BEGIN_VIS, false, sizeof(bool) * _BFS_BEGIN_QSZ); \
	_BFS_BEGIN_VIS[_BFS_BEGIN_START] = true;

#define QUEUE_POP(_QUEUE_POP_Q, _QUEUE_POP_H, _QUEUE_POP_QSZ) \
	_QUEUE_POP_Q[_QUEUE_POP_H];                               \
	_QUEUE_POP_H += _QUEUE_POP_H < _QUEUE_POP_QSZ;

#define QUEUE_PUSH(                                  \
	_QUEUE_PUSH_Q,                                   \
	_QUEUE_PUSH_T,                                   \
	_QUEUE_PUSH_QSZ,                                 \
	_QUEUE_PUSH_ELEM                                 \
)                                                    \
	_QUEUE_PUSH_Q[_QUEUE_PUSH_T] = _QUEUE_PUSH_ELEM; \
	_QUEUE_PUSH_T += _QUEUE_PUSH_T < _QUEUE_PUSH_QSZ;

#endif

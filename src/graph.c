#include <string.h>

#include "graph.h"
#include "safealloc.h"
#include "vector.h"

struct Graph
graph_make(void)
{
	return (struct Graph){
		.nodes = calloc_s(MAX_NODES, sizeof(struct Node)),
		.n_nodes = 0,
	};
}

void
graph_delete(struct Graph *g)
{
	for (int i = 0; i < MAX_NODES; i++) {
		free_s(g->nodes[i].adj);
		free_s(g->nodes[i].cmd);
		free_s(g->nodes[i].filename);
	}
	free_s(g->nodes);
	return;
}

/*
 * Add a node to an adjlist, ignoring duplicates.
 * BUG: linear search is making this EXTREMELY slow.
 */
static void
adjlist_add(struct Node *from, size_t to)
{
	// Naive linear search because yes
	// Ensure no duplicates
	if (from->adj) {
		for (size_t *c = from->adj; c < from->adj + from->len; c++) {
			if (*c == to) {
				return;
			}
		}
	}
	from->adj = realloc_s(from->adj, sizeof(*(from->adj)) * (from->len + 1));
	from->adj[from->len] = to;
	from->len += 1;
	return;
}

void
graph_add_edge(struct Graph *g, size_t from, size_t to)
{
	struct Node *afrom = g->nodes + from;
	g->n_nodes += !g->nodes[from].exists;
	g->n_nodes += !g->nodes[to].exists;
	adjlist_add(afrom, to);
	g->nodes[from].exists = 1;
	g->nodes[to].exists = 1;
	return;
}

void
graph_add_meta(
	struct Graph *g,
	size_t at,
	const char *cmd,
	const char *filename
)
{
	struct Node *afrom = g->nodes + at;

	afrom->cmd = malloc_s((strlen(cmd) + 1) * sizeof(char));
	strcpy(afrom->cmd, cmd);
	afrom->filename = malloc_s((strlen(filename) + 1) * sizeof(char));
	strcpy(afrom->filename, filename);
	return;
}

void
graph_add_target(
	struct Graph *g,
	size_t target,
	size_t *deps,
	size_t n_deps,
	const char *cmd,
	const char *filename
)
{
	graph_add_meta(g, target, cmd, filename);
	// Heh apparently not doing this is UB
	if (!deps) {
		return;
	}
	for (size_t *d = deps; d < deps + n_deps; d++) {
		graph_add_edge(g, target, *d);
	}
	return;
}

void
graph_copy(struct Graph *src, struct Graph *dest, size_t start, bool invert)
{
	BFS_BEGIN(queue, MAX_NODES, h, t, visited, start);

	while (h != t) {
		size_t c = QUEUE_POP(queue, h, MAX_NODES);
		graph_add_meta(dest, c, src->nodes[c].cmd, src->nodes[c].filename);

		if (!src->nodes[c].adj) {
			continue;
		}

		for (size_t *n = src->nodes[c].adj;
		     n < src->nodes[c].adj + src->nodes[c].len;
		     n++) {

			if (invert) {
				graph_add_edge(dest, *n, c); // invert graph
			} else {
				graph_add_edge(dest, c, *n);
			}

			if (!visited[*n]) {
				visited[*n] = true;
				QUEUE_PUSH(queue, t, MAX_NODES, *n);
			}
		}
	}

	return;
}

void
toposort(struct Graph *g, size_t c, struct Vec_size_t *tsorted, char *visited)
{
	// Enqueue dependencies
	struct Node g_nde = g->nodes[c];
	if (g_nde.adj) {
		for (size_t *n = g_nde.adj; n < g_nde.adj + g_nde.len; n++) {
			if (!visited[*n]) {
				visited[*n] = true;
				toposort(g, *n, tsorted, visited);
			}
		}
	}
	VEC_PUSH(*tsorted, c);
}

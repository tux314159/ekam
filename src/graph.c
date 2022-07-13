#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "safealloc.h"

Graph graph_make(size_t max_nodes)
{
    return calloc_s(max_nodes, sizeof(ARow));
}

void graph_add_edge(Graph graph, size_t from, size_t to)
{
    ARow *afrom  = graph + from;
    afrom->adj = realloc_s(afrom->adj, sizeof(*(afrom->adj)) * afrom->len + 1);
    afrom->adj[afrom->len] = to;
    afrom->len += 1;
    return;
}

void graph_bfs_into(Graph src, Graph dest, size_t start)
{
    size_t queue[MAX_NODES];
    size_t h = 0, t = 1;
    queue[0] = start;

    bool visited[MAX_NODES] = {0};
    memset(visited, false, sizeof(bool) * MAX_NODES);
    visited[start] = true;

    while (h != t) {
        size_t c = queue[h];
        h = h < MAX_NODES ? h + 1 : 0;
        for (size_t *n = src[c].adj; n < src[c].adj + src[c].len; c++) {
            graph_add_edge(dest, c, *n);
            if (!visited[*n]) {
                visited[*n] = true;
                queue[t] = *n;
                t = t < MAX_NODES ? t + 1 : 0;
            }
        }
    }

    return;
}

void graph_buildpartial(Graph src, Graph dest, size_t *starts, size_t n_starts)
{
    for (size_t i = 0; i < n_starts; i++) {
        graph_bfs_into(src, dest, starts[i]);
    }
    return;
}

size_t *graph_toposort(Graph src, size_t *starts, size_t n_starts)
    size_t *degrees = calloc_s(src.n_nodes, sizeof(src.adjlist));
    size_t queue[MAX_NODES];
    size_t h = 0, t = n_starts;
    memcpy(queue, starts, n_starts * sizeof(*starts));
    queue[0] = starts[0];

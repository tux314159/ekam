#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "safealloc.h"

Graph graph_make(void)
{
    return calloc_s(MAX_NODES, sizeof(ARow));
}

void graph_delete(Graph g)
{
    for (int i = 0; i < MAX_NODES; i++) {
        free(g[i].adj);
    }
    free(g);
    return;
}

void adjlist_add(struct ARow *from, size_t to) {
    // naive linear search because yes
    // ensure no duplicates
    for (size_t *c = from->adj; c < from->adj + from->len; c++) {
        if (*c == to) {
            return;
        }
    }
    from->adj = realloc_s(from->adj, sizeof(*(from->adj)) * (from->len + 1));
    from->adj[from->len] = to;
    from->len += 1;
    return;
}

void graph_add_edge(Graph graph, size_t from, size_t to)
{
    ARow *afrom  = graph + from;
    adjlist_add(afrom, to);
    return;
}

void graph_bfs_into(Graph src, Graph dest, size_t start)
{
    size_t queue[MAX_NODES];
    size_t h = 0, t = 1;
    queue[0] = start;

    bool visited[MAX_NODES];
    memset(visited, false, sizeof(bool) * MAX_NODES);
    visited[start] = true;

    while (h != t) {
        size_t c = queue[h];
        h = h < MAX_NODES ? h + 1 : 0;
        for (size_t *n = src[c].adj; n < src[c].adj + src[c].len; n++) {
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

void graph_execute(Graph g)
{
    // TODO: parallelise

    size_t queue[MAX_NODES];
    size_t h = 0, t = 1;
    queue[0] = 0;

    bool visited[MAX_NODES];
    memset(visited, false, sizeof(bool) * MAX_NODES);
    visited[0] = true;

    while (h != t) {
        size_t c = queue[h];
        h = h < MAX_NODES ? h + 1 : 0;
        for (size_t *n = g[c].adj; n < g[c].adj + g[c].len; n++) {
            if (!visited[*n]) {
                visited[*n] = true;
                queue[t] = *n;
                t = t < MAX_NODES ? t + 1 : 0;
            }
        }
    }
}

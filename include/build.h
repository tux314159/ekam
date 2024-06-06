#ifndef INCLUDE_BUILD
#define INCLUDE_BUILD

#include "graph.h"

/*
 * Build the partial graph containg all nodes in need of updating
 * starting from some starting points. Prunes nodes based on update
 * time comparisons.
 */
void
cons_partgraph(struct Graph *src, struct Graph *dest, size_t start);

/*
 * Execute the build described by a graph, possibly in parallel.
 */
void
build_graph(struct Graph *g, int max_childs);

#endif

#ifndef INCLUDE_GRAPH
#define INCLUDE_GRAPH

#include <sys/types.h>

#define MAX_NODES 1024
typedef struct ARow {  // row in adjlist
    size_t *adj;
    size_t len;
} ARow;

typedef struct Node {  // probably kept in a global, indices are stored in the adjlist
    char *fname;
} Node;

typedef struct Graph {
    ARow *adjlist;
    size_t n_nodes;
} Graph;

Graph graph_make(size_t max_nodes);
void graph_add_edge(Graph graph, size_t from, size_t to);
void graph_bfs_into(Graph src, Graph dest, size_t start);
void graph_buildpartial(Graph src, Graph dest, size_t *starts, size_t n_starts);

#endif

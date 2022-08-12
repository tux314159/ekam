#ifndef INCLUDE_GRAPH
#define INCLUDE_GRAPH

#include <sys/types.h>

#define MAX_NODES 1024

struct ARow {
    size_t *adj;
    size_t len;
};

struct Node {
    char *fname;
    char *cmd;
};

struct Graph {
    struct ARow *graph;
    struct Node *nodes;
    size_t n_nodes;
};

struct Graph graph_make(void);
void graph_delete(struct Graph g);
void adjlist_add(struct ARow *from, size_t to);
void graph_add_edge(struct Graph graph, size_t from, size_t to);
void graph_bfs_into(struct Graph src, struct Graph dest, size_t start);
void graph_buildpartial(struct Graph src, struct Graph dest, size_t *starts, size_t n_starts);
void graph_execute(struct Graph g);

#endif

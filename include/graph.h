#ifndef INCLUDE_GRAPH
#define INCLUDE_GRAPH

#include <sys/types.h>

#define MAX_NODES 1024

struct Node {
    size_t *adj;
    size_t len;
    char *cmd;
};

struct Graph {
    struct Node *graph;
    size_t n_nodes;
};

struct Graph graph_make(void);
void graph_delete(struct Graph g);
void adjlist_add(struct Node *from, size_t to);
void graph_add_edge(struct Graph graph, size_t from, size_t to);
void graph_add_rule(struct Graph graph, size_t at, const char *cmd);
void graph_buildpartial(struct Graph src, struct Graph dest, size_t *starts, size_t n_starts);
void graph_execute(struct Graph g, size_t start);

#endif

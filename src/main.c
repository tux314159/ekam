#include <stdio.h>
#include "graph.h"

int main(void)
{
    Graph g = graph_make();
    graph_add_edge(g, 0, 1);
    graph_add_edge(g, 0, 2);
    graph_add_edge(g, 1, 3);
    graph_add_edge(g, 1, 4);
    graph_add_edge(g, 4, 5);
    graph_add_edge(g, 2, 5);
    graph_execute(g);
    graph_delete(g);
    return 0;
}

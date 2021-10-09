#include <graph.h>

#include <stdio.h>

int main(void)
{
    struct adjlist_t *al;
    struct adjlist_t *blank;
    size_t *t;

    al = calloc(1, sizeof(*al));
    blank = calloc(1, sizeof(*blank));
    t = alloca(444);
    t[0] = 1;
    t[1] = 3;
    t[2] = 4;

    for (size_t i = 0; i < 9; i++) {
        g_add_node(al, i);
    }

    g_add_edge(al, 0, 5);
    g_add_edge(al, 1, 5);
    g_add_edge(al, 1, 6);
    g_add_edge(al, 2, 6);
    g_add_edge(al, 3, 7);
    g_add_edge(al, 4, 7);
    g_add_edge(al, 5, 8);
    g_add_edge(al, 6, 8);
    g_add_edge(al, 7, 8);

    g_construct_partial(al, blank, 3, t);

    for (struct node_t *c = blank->adjlist; c != NULL; c = c->hh.next) {
        for (size_t *p = c->adj; p < c->adj + c->conn_n; p++) {
            printf("%lu -> %lu\n", c->id, *p);
        }
    }

    return 0;
}

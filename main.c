#include "ekam"

int main(int argc, char **argv)
{
    struct Graph g = graph_make();
    struct Graph pg = graph_make();
    graph_add_rule(&g, 0, "echo at 0; sleep 0.5");
    graph_add_rule(&g, 1, "echo at 1; sleep 0.5");
    graph_add_rule(&g, 2, "echo at 2; sleep 0.5");
    graph_add_rule(&g, 3, "echo at 3; sleep 0.5");
    graph_add_rule(&g, 4, "echo at 4; sleep 0.5");
    graph_add_rule(&g, 5, "echo at 5; sleep 0.5");
    graph_add_rule(&g, 6, "echo at 6; sleep 0.5");

    graph_add_edge(&g, 1, 0);
    graph_add_edge(&g, 2, 0);
    graph_add_edge(&g, 2, 6);
    graph_add_edge(&g, 3, 1);
    graph_add_edge(&g, 4, 1);
    graph_add_edge(&g, 5, 4);
    graph_add_edge(&g, 5, 2);

    graph_buildpartial(&g, &pg, (size_t[]){ 5 }, 1);
    graph_execute(&pg, 5, argc == 1 ? 1 : atoi(argv[1]));
    graph_delete(&pg);
    graph_delete(&g);
    return 0;
}

#include "ekam"
#define NULL 0

int main(int argc, char **argv)
{
    struct Graph g  = graph_make();
    struct Graph pg = graph_make();
    graph_add_target(&g, 0, (size_t[]){1, 2}, 2, "echo at 0; sleep 0.5");
    graph_add_target(&g, 1, (size_t[]){3, 4}, 2, "echo at 1; sleep 0.5");
    graph_add_target(&g, 2, (size_t[]){5}, 1, "echo at 2; sleep 0.5");
    graph_add_target(&g, 3, NULL, 0, "echo at 3; sleep 0.5");
    graph_add_target(&g, 4, (size_t[]){5}, 1, "echo at 4; sleep 0.5");
    graph_add_target(&g, 5, NULL, 0, "echo at 5; sleep 0.5");
    graph_add_target(&g, 6, (size_t[]){2, 8, 9}, 3, "echo at 6; sleep 0.5");
    graph_add_target(&g, 7, (size_t[]){1, 0, 6}, 3, "echo at 7; sleep 0.5");
    graph_add_target(&g, 8, (size_t[]){5}, 1, "echo at 8; sleep 0.5");
    graph_add_target(&g, 9, (size_t[]){5}, 1, "echo at 9; sleep 0.5");

    graph_buildpartial(&g, &pg, (size_t[]){5}, 1);
    graph_execute(&pg, 5, argc == 1 ? 1 : atoi(argv[1]));
    graph_delete(&pg);
    graph_delete(&g);
    return 0;
}

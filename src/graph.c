#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "graph.h"
#include "safealloc.h"

static void msleep(long msec)
{
    struct timespec ts;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    nanosleep(&ts, &ts);
    return;
}

struct Graph graph_make(void)
{
    return (struct Graph){
        .graph   = calloc_s(MAX_NODES, sizeof(struct Node)),
        .rgraph  = calloc_s(MAX_NODES, sizeof(struct Node)),
        .n_nodes = 0,
    };
}

void graph_delete(struct Graph *g)
{
    for (int i = 0; i < MAX_NODES; i++) {
        free(g->graph[i].adj);
        free(g->rgraph[i].adj);
    }
    free(g->graph);
    free(g->rgraph);
    return;
}

void adjlist_add(struct Node *from, size_t to)
{
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

void graph_add_edge(struct Graph *g, size_t from, size_t to)
{
    struct Node *afrom  = g->graph + from;
    struct Node *rafrom = g->rgraph + to;
    adjlist_add(afrom, to);
    adjlist_add(rafrom, from);
    g->n_nodes++;
    return;
}

void graph_add_rule(struct Graph *g, size_t at, const char *cmd)
{
    struct Node *afrom = g->graph + at;
    afrom->cmd         = malloc((strlen(cmd) + 1) * sizeof(char));
    strcpy(afrom->cmd, cmd);
    return;
}

static void _bfs_copy(struct Graph *src, struct Graph *dest, size_t start)
{
    size_t queue[MAX_NODES];
    size_t h = 0, t = 1;
    queue[0] = start;

    bool visited[MAX_NODES];
    memset(visited, false, sizeof(bool) * MAX_NODES);
    visited[start] = true;

    while (h != t) {
        size_t c = queue[h];
        h        = h < MAX_NODES ? h + 1 : 0;
        graph_add_rule(dest, c, src->graph[c].cmd);

        for (size_t *n = src->graph[c].adj;
             n < src->graph[c].adj + src->graph[c].len;
             n++) {

            graph_add_edge(dest, c, *n);
            if (!visited[*n]) {
                visited[*n] = true;
                queue[t]    = *n;
                t           = t < MAX_NODES ? t + 1 : 0;
            }
        }
    }

    return;
}

void graph_buildpartial(
    struct Graph *src,
    struct Graph *dest,
    size_t       *starts,
    size_t        n_starts)
{
    for (size_t i = 0; i < n_starts; i++) {
        _bfs_copy(src, dest, starts[i]);
    }
    return;
}

void graph_execute(struct Graph *g, size_t start, int max_childs)
{
    size_t queue[MAX_NODES];
    size_t h = 0, t = 1;
    queue[0] = start;

    bool   visited[MAX_NODES];
    size_t tsorted[MAX_NODES];
    memset(visited, false, sizeof(bool) * MAX_NODES);
    memset(tsorted, 0, sizeof(size_t) * MAX_NODES);
    visited[start] = true;

    // toposort
    size_t i = 0;
    while (h != t) {
        size_t c = queue[h];
        h += h < MAX_NODES;

        tsorted[i++] = c;

        // enqueue dependants
        for (size_t *n = g->graph[c].adj; n < g->graph[c].adj + g->graph[c].len;
             n++) {
            if (!visited[*n]) {
                visited[*n] = true;
                queue[t]    = *n;
                t += t < MAX_NODES;
            }
        }
    }

    // set up shm
    const char *shm_name = "/ekam";
    const size_t shm_sz = sizeof(int) + sizeof(int) * MAX_NODES;
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    shm_unlink(shm_name);
    ftruncate(shm_fd, (off_t)shm_sz);
    int *n_childs = mmap(NULL, shm_sz, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    int *processed = n_childs + 1;
    memset(processed, 0, sizeof(int) * MAX_NODES);

    size_t cnt = 0;
    for (;;) {
        wait(NULL); // clean up zombies
        msleep(10);

        if (cnt == g->n_nodes) {
            // processed all nodes, we are done
            break;
        }

        for (size_t *c = tsorted; c < tsorted + g->n_nodes; c++) {
            if (processed[*c]) {
                continue;
            }

            if (*n_childs >= max_childs) {
                break;
            }

            bool deps_sat = true;
            for (size_t *d = g->rgraph[*c].adj;
                 d < g->rgraph[*c].adj + g->rgraph[*c].len;
                 d++) {
                deps_sat &= (processed[*d] == 2);
            }

            if (deps_sat) {
                (*n_childs)++;
                processed[*c] = 1;
                cnt++;
                pid_t pid = fork();
                if (!pid) {
                    processed[*c] = 1;
                    system(g->graph[*c].cmd);
                    processed[*c] = 2;
                    (*n_childs)--;
                    _exit(0);
                }
            }
        }
    }
    munmap(n_childs, shm_sz);
}

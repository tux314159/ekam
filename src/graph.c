#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <util/debug.h>

#include "graph.h"
#include "safealloc.h"

struct Graph
graph_make(void)
{
	return (struct Graph){
		.graph   = calloc_s(MAX_NODES, sizeof(struct Node)),
		.rgraph  = calloc_s(MAX_NODES, sizeof(struct Node)),
		.n_nodes = 0,
	};
}

void
graph_delete(struct Graph *g)
{
	for (int i = 0; i < MAX_NODES; i++) {
		free(g->graph[i].adj);
		free(g->rgraph[i].adj);
	}
	free(g->graph);
	free(g->rgraph);
	return;
}

void
adjlist_add(struct Node *from, size_t to)
{
	// Naive linear search because yes
	// Ensure no duplicates
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

void
graph_add_edge(struct Graph *g, size_t from, size_t to)
{
	if (!(from && to)) {
	}

	struct Node *afrom  = g->graph + from;
	struct Node *rafrom = g->rgraph + to;
	g->n_nodes += g->graph[from].len == 0 && g->rgraph[from].len == 0;
	g->n_nodes += g->graph[to].len == 0 && g->rgraph[to].len == 0;
	adjlist_add(afrom, to);
	adjlist_add(rafrom, from);
	return;
}

void
graph_add_rule(struct Graph *g, size_t at, const char *cmd)
{
	struct Node *afrom = g->graph + at;
	afrom->cmd         = malloc_s((strlen(cmd) + 1) * sizeof(char));
	strcpy(afrom->cmd, cmd);
	return;
}

void
graph_add_target(
	struct Graph *g,
	size_t        target,
	size_t       *deps,
	size_t        n_deps,
	const char   *cmd
)
{
	graph_add_rule(g, target, cmd);
	for (size_t *d = deps; d < deps + n_deps; d++) {
		graph_add_edge(g, *d, target);
	}
	return;
}

static void
_bfs_copy(struct Graph *src, struct Graph *dest, size_t start)
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

void
graph_buildpartial(
	struct Graph *src,
	struct Graph *dest,
	size_t       *starts,
	size_t        n_starts
)
{
	graph_add_rule(dest, 0, "");
	for (size_t *i = starts; i < starts + n_starts; i++) {
		_bfs_copy(src, dest, *i);
		graph_add_edge(dest, 0, *i);
	}
	return;
}

void
graph_execute(struct Graph *g, int max_childs)
{
	size_t queue[MAX_NODES];
	size_t h = 0, t = 1;
	queue[0] = 0;

	bool   visited[MAX_NODES];
	size_t tsorted[MAX_NODES];
	memset(visited, false, sizeof(bool) * MAX_NODES);
	memset(tsorted, 0, sizeof(size_t) * MAX_NODES);
	visited[0] = true;

	// Toposort
	size_t i = 0;
	while (h != t) {
		size_t c = queue[h];
		h += h < MAX_NODES;

		tsorted[i++] = c;

		// Enqueue dependants
		for (size_t *n = g->graph[c].adj; n < g->graph[c].adj + g->graph[c].len;
		     n++) {
			if (!visited[*n]) {
				visited[*n] = true;
				queue[t]    = *n;
				t += t < MAX_NODES;
			}
		}
	}

	// Set up shm
	const size_t shm_sz  = sizeof(sem_t) * 2 + sizeof(int) * MAX_NODES;
	int          zero_fd = open("/dev/zero", O_RDWR);
	void        *mem     = mmap(
        NULL,
        shm_sz,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        zero_fd,
        0
    ); // For clarity
	sem_t *n_childs = mem;
	sem_t *plock    = (sem_t *)mem + 1;
	sem_init(n_childs, 1, 0);
	sem_init(plock, 1, (unsigned)max_childs);

	int *processed = (int *)((sem_t *)mem + 2);
	memset(processed, 0, sizeof(int) * MAX_NODES);

	int    n_c;
	size_t cnt = 0;
	for (;;) {
		while (waitpid(-1, NULL, WNOHANG) > 0)
			; // Reap dead children
		sem_getvalue(plock, &n_c);
		sem_wait(plock);

		if (cnt == g->n_nodes) {
			// Processed all nodes, we are done
			break;
		}

		for (size_t *c = tsorted; c < tsorted + g->n_nodes; c++) {
			if (processed[*c]) {
				continue;
			}

			sem_getvalue(n_childs, &n_c);
			if (n_c >= max_childs) {
				break;
			}

			bool deps_sat = true;
			for (size_t *d = g->rgraph[*c].adj;
			     d < g->rgraph[*c].adj + g->rgraph[*c].len;
			     d++) {
				deps_sat &= (processed[*d] == 2);
			}

			if (deps_sat) {
				sem_post(n_childs);
				processed[*c] = 1;
				cnt++;
				if (!fork()) {
					processed[*c] = 1;
					system(g->graph[*c].cmd);
					processed[*c] = 2;
					sem_wait(n_childs);
					sem_post(plock);
					_exit(0);
				}
			}
		}
	}
	sem_destroy(n_childs);
	sem_destroy(plock);
	munmap(mem, shm_sz);
}

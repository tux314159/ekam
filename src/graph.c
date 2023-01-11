#include <dirent.h>
#include <fcntl.h>
#include <search.h>
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

#include "datastructs.h"
#include "graph.h"
#include "safealloc.h"

// We're doing BFS everywhere - why not macro?
#define BFS_BEGIN(                                                \
	_BFS_BEGIN_Q,                                                 \
	_BFS_BEGIN_QSZ,                                               \
	_BFS_BEGIN_H,                                                 \
	_BFS_BEGIN_T,                                                 \
	_BFS_BEGIN_VIS,                                               \
	_BFS_BEGIN_START                                              \
)                                                                 \
	size_t _BFS_BEGIN_Q[_BFS_BEGIN_QSZ];                          \
	size_t _BFS_BEGIN_H = 0, _BFS_BEGIN_T = 1;                    \
	_BFS_BEGIN_Q[0] = _BFS_BEGIN_START;                           \
	char _BFS_BEGIN_VIS[_BFS_BEGIN_QSZ];                          \
	memset(_BFS_BEGIN_VIS, false, sizeof(bool) * _BFS_BEGIN_QSZ); \
	_BFS_BEGIN_VIS[_BFS_BEGIN_START] = true;

#define QUEUE_POP(_QUEUE_POP_Q, _QUEUE_POP_H, _QUEUE_POP_QSZ) \
	_QUEUE_POP_Q[_QUEUE_POP_H];                               \
	_QUEUE_POP_H += _QUEUE_POP_H < MAX_NODES;

#define QUEUE_PUSH(                                  \
	_QUEUE_PUSH_Q,                                   \
	_QUEUE_PUSH_T,                                   \
	_QUEUE_PUSH_QSZ,                                 \
	_QUEUE_PUSH_ELEM                                 \
)                                                    \
	_QUEUE_PUSH_Q[_QUEUE_PUSH_T] = _QUEUE_PUSH_ELEM; \
	_QUEUE_PUSH_T += _QUEUE_PUSH_T < _QUEUE_PUSH_QSZ;

struct Graph
graph_make(void)
{
	return (struct Graph){
		.nodes   = calloc_s(MAX_NODES, sizeof(struct Node)),
		.n_nodes = 0,
	};
}

void
graph_delete(struct Graph *g)
{
	for (int i = 0; i < MAX_NODES; i++) {
		free(g->nodes[i].adj);
		free(g->nodes[i].cmd);
		free(g->nodes[i].filename);
	}
	free(g->nodes);
	return;
}

/*
 * Add a node to an adjlist, ignoring duplicates.
 * BUG: linear search is making this EXTREMELY slow.
 */
static void
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
	struct Node *afrom = g->nodes + from;
	g->n_nodes += !g->nodes[from].exists;
	g->n_nodes += !g->nodes[to].exists;
	adjlist_add(afrom, to);
	g->nodes[from].exists = 1;
	g->nodes[to].exists   = 1;
	return;
}

void
graph_add_meta(
	struct Graph *g,
	size_t        at,
	const char   *cmd,
	const char   *filename
)
{
	struct Node *afrom = g->nodes + at;

	afrom->cmd = malloc_s((strlen(cmd) + 1) * sizeof(char));
	strcpy(afrom->cmd, cmd);
	afrom->filename = malloc_s((strlen(filename) + 1) * sizeof(char));
	strcpy(afrom->filename, filename);
	return;
}

void
graph_add_target(
	struct Graph *g,
	size_t        target,
	size_t       *deps,
	size_t        n_deps,
	const char   *cmd,
	const char   *filename
)
{
	graph_add_meta(g, target, cmd, filename);
	for (size_t *d = deps; d < deps + n_deps; d++) {
		graph_add_edge(g, target, *d);
	}
	return;
}

static void
_bfs_copy(struct Graph *src, struct Graph *dest, size_t start, bool invert)
{
	BFS_BEGIN(queue, MAX_NODES, h, t, visited, start);

	while (h != t) {
		size_t c = QUEUE_POP(queue, h, MAX_NODES);
		graph_add_meta(dest, c, src->nodes[c].cmd, src->nodes[c].filename);

		for (size_t *n = src->nodes[c].adj;
		     n < src->nodes[c].adj + src->nodes[c].len;
		     n++) {

			if (invert) {
				graph_add_edge(dest, *n, c); // invert graph
			} else {
				graph_add_edge(dest, c, *n);
			}

			if (!visited[*n]) {
				visited[*n] = true;
				QUEUE_PUSH(queue, t, MAX_NODES, *n);
			}
		}
	}

	return;
}

void
graph_buildpartial(struct Graph *src, struct Graph *dest, size_t start)
{
	// BFS on source to find every vertex in direct need
	// of updating reachable from start.
	struct Vec_size_t needupd;
	VEC_INIT(needupd);
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, start);
		while (h != t) {
			size_t      c     = QUEUE_POP(queue, h, MAX_NODES);
			struct Node c_nde = src->nodes[c];

			// Read mtime
			struct stat csb;
			int         csr = stat(c_nde.filename, &csb);

			// If we do not exist, we definitely need some updating.
			if (csr == -1) {
				VEC_PUSH(needupd, c);
			}

			for (size_t *n = c_nde.adj; n < c_nde.adj + c_nde.len; n++) {
				// We want every pair, so we do this before visited check;
				// at most we get one duplicate (I think) (I hope)
				struct stat nsb;
				int         nsr = stat(src->nodes[*n].filename, &nsb);

				// We need to be updated if our direct dependency needs it.
				// If it does not exist, it will exist, and therefore we will
				// be reached later on anyway, so no need to push ourselves.
				if (nsr != -1) {
					if (nsb.st_mtim.tv_sec > csb.st_mtim.tv_sec ||
					    (nsb.st_mtim.tv_sec == csb.st_mtim.tv_sec &&
					     nsb.st_mtim.tv_nsec >= csb.st_mtim.tv_nsec)) {
						VEC_PUSH(needupd, *n);
					}
				}

				if (visited[*n]) {
					continue;
				}
				visited[*n] = 1;
				QUEUE_PUSH(queue, t, MAX_NODES, *n);
			}
		}
	}

	// Build inverted graph with all deps. Essentially - reverse edges.
	struct Graph inv = graph_make();
	_bfs_copy(src, &inv, start, true);

	// Do multisource BFS with nodes needing update as starting points
	// and therefore construct dest.
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, 0);

		memcpy(queue, needupd.arr, needupd.sz * sizeof(*needupd.arr));
		for (size_t i = 0; i < needupd.sz; i++)
			visited[needupd.arr[i]] = true;
		t = needupd.sz;

		while (h != t) {
			size_t      c     = QUEUE_POP(queue, h, MAX_NODES);
			struct Node c_nde = inv.nodes[c];
			// Copy metadata over
			graph_add_meta(dest, c, src->nodes[c].cmd, src->nodes[c].filename);

			for (size_t *n = c_nde.adj; n < c_nde.adj + c_nde.len; n++) {
				graph_add_edge(dest, *n, c); // add to dest (uninvert graph)
				if (visited[*n]) {
					continue;
				}
				visited[*n] = true;

				QUEUE_PUSH(queue, t, MAX_NODES, *n);
			}
		}
	}
	graph_add_edge(dest, 0, start); // 0 node

	free(needupd.arr);
	return;
}

static void
toposort(struct Graph *g, size_t c, struct Vec_size_t *tsorted, char *visited)
{
	// Enqueue dependencies
	struct Node g_nde = g->nodes[c];
	for (size_t *n = g_nde.adj; n < g_nde.adj + g_nde.len; n++) {
		if (!visited[*n]) {
			visited[*n] = true;
			toposort(g, *n, tsorted, visited);
		}
	}
	VEC_PUSH(*tsorted, c);
}

void
graph_execute(struct Graph *g, int max_childs)
{
	// DFS toposort
	struct Vec_size_t tsorted_s;
	VEC_INIT(tsorted_s);
	char visited[MAX_NODES];
	memset(visited, 0, MAX_NODES);
	visited[0] = 1;
	toposort(g, 0, &tsorted_s, visited);
	size_t *tsorted = tsorted_s.arr;

	// Reverse toposorted array - since graph is reversed.
	for (size_t i = 0; i < g->n_nodes / 2; i++) {
		size_t tmp                  = tsorted[i];
		tsorted[i]                  = tsorted[g->n_nodes - i - 1];
		tsorted[g->n_nodes - i - 1] = tmp;
	}

	// Time to finally start executing shit!

	// Set up shm
	const size_t shm_sz = sizeof(sem_t) + sizeof(int) + sizeof(int) * MAX_NODES;
	int          zero_fd = open("/dev/zero", O_RDWR);
	void        *mem =
		mmap(NULL, shm_sz, PROT_READ | PROT_WRITE, MAP_SHARED, zero_fd, 0);

	sem_t *plock = mem;
	sem_init(plock, 1, (unsigned)max_childs);

	int *n_childs = (int *)((sem_t *)mem + 1);
	*n_childs     = 0;

	int *processed = (int *)((sem_t *)mem + 1) + 1;
	memset(processed, 0, sizeof(int) * MAX_NODES);

	size_t cnt = 0;
	for (;;) {
		while (waitpid(-1, NULL, WNOHANG) > 0)
			; // Reap dead children
		int val;
		sem_getvalue(plock, &val);
		sem_wait(plock);
		sem_getvalue(plock, &val);

		if (cnt == g->n_nodes) {
			// Processed all nodes, we are done
			break;
		}

		for (size_t *c = tsorted; c < tsorted + g->n_nodes; c++) {
			if (processed[*c]) {
				continue;
			}

			if (*n_childs >= max_childs) {
				break;
			}

			// Check if all dependencies are satisfied
			bool deps_sat = true;
			for (size_t *d = g->nodes[*c].adj;
			     d < g->nodes[*c].adj + g->nodes[*c].len;
			     d++) {
				deps_sat &= (processed[*d] == 2);
			}

			if (deps_sat) {
				(*n_childs)++;
				processed[*c] = 1;
				cnt++;
				// Fork and run command
				if (!fork()) {
					processed[*c] = 1;
					system(g->nodes[*c].cmd);
					processed[*c] = 2;
					(*n_childs)--;
					sem_post(plock);
					_exit(0);
				}
			}
		}
	}
	sem_destroy(plock);
	munmap(mem, shm_sz);
	VEC_KILL(tsorted_s);
}

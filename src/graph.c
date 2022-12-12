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
		.nodes    = calloc_s(MAX_NODES, sizeof(struct Node)),
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
	// Build inverted graph with all deps. Essentially - reverse edges.
	struct Graph inv = graph_make();
	_bfs_copy(src, &inv, start, true);

	// BFS on source to find leaves.
	size_t *leaves   = malloc_s(sizeof(*leaves));
	size_t  n_leaves = 0;
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, start);
		while (h != t) {
			size_t c = QUEUE_POP(queue, h, MAX_NODES);
			{ // no deps, i.e. leaf
				leaves[n_leaves++] = c;
				leaves = realloc_s(leaves, sizeof(*leaves) * (n_leaves + 1));
			}

			for (size_t *n = src->nodes[c].adj;
			     n < src->nodes[c].adj + src->nodes[c].len;
			     n++) {
				if (!visited[*n]) {
					visited[*n] = true;
					QUEUE_PUSH(queue, t, MAX_NODES, *n);
				}
			}
		}
	}

	// BFS, on inverted graph, read mtimes, check if node needs
	// to be updated.
	char needupd[MAX_NODES];
	memset(needupd, false, MAX_NODES);
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, 0);

		// Push all leaves into the queue first - we want to simulate a
		// node on top of all of these.
		memcpy(queue, leaves, n_leaves * sizeof(*leaves));
		for (size_t i = 0; i < n_leaves; i++)
			visited[leaves[i]] = true;
		t = n_leaves;

		while (h != t) {
			size_t c = QUEUE_POP(queue, h, MAX_NODES);
			// Copy metadata over
			graph_add_meta(dest, c, src->nodes[c].cmd, src->nodes[c].filename);

			// Read dep mtime
			struct stat csb;
			int         csr = stat(inv.nodes[c].filename, &csb);

			for (size_t *n = inv.nodes[c].adj;
			     n < inv.nodes[c].adj + inv.nodes[c].len;
			     n++) {
				// Read codep mtime
				struct stat sb;
				int         sr = stat(inv.nodes[*n].filename, &sb);

				// Codep needs to be updated if dep was modified
				// at a later or equal time. If dep needs to be
				// updated so does codep.
				needupd[c] = needupd[c] || needupd[*n] || sr == -1 ||
				             csr == -1 ||
				             sb.st_mtim.tv_sec > csb.st_mtim.tv_sec ||
				             (sb.st_mtim.tv_sec == csb.st_mtim.tv_sec &&
				              sb.st_mtim.tv_nsec >= csb.st_mtim.tv_nsec) ||
				             needupd[*n];
				if (needupd[c])
					graph_add_edge(dest, *n, c); // add to dest (uninvert graph)

				if (!visited[*n]) {
					visited[*n] = true;
					QUEUE_PUSH(queue, t, MAX_NODES, *n);
				}
			}

			if (needupd[c]) {
				debugpf("Node %ld needs updating", c);
			} else {
				debugpf("Node %ld does not need updating", c);
			}
		}
	}
	graph_add_edge(dest, 0, start); // 0 node

	free(leaves);
	return;
}

struct __vec {
	size_t sz;
	size_t *arr;
};

static void toposort(struct Graph *g, size_t c, struct __vec *tsorted, char *visited)
{
	// Enqueue dependencies
	for (size_t *n = g->nodes[c].adj; n < g->nodes[c].adj + g->nodes[c].len;
			n++) {
		if (!visited[*n]) {
			visited[*n] = true;
			toposort(g, *n, tsorted, visited);
		}
	}
	tsorted->arr[tsorted->sz++] = c;
}

void
graph_execute(struct Graph *g, int max_childs)
{
	// DFS toposort
	struct __vec tsorted_s;
	tsorted_s.sz = 0;
	tsorted_s.arr = calloc(MAX_NODES, sizeof(*tsorted_s.arr));
	char visited[MAX_NODES];
	memset(visited, 0, MAX_NODES);
	visited[0] = 1;
	toposort(g, 0, &tsorted_s, visited);
	size_t *tsorted = tsorted_s.arr;

	// Reverse toposorted array - it was actually done in reverse
	for (size_t i = 0; i < g->n_nodes / 2; i++) {
		size_t tmp                  = tsorted[i];
		tsorted[i]                  = tsorted[g->n_nodes - i - 1];
		tsorted[g->n_nodes - i - 1] = tmp;
	}

	// Time to finally start executing shit!

	// Set up shm
	// XXX: is this portable? Will sem_t ever be smaller than int?
	const size_t shm_sz = sizeof(int) + (sizeof(sem_t) - sizeof(int)) +
	                      sizeof(sem_t) + sizeof(int) * MAX_NODES;
	int   zero_fd = open("/dev/zero", O_RDWR);
	void *mem =
		mmap(NULL, shm_sz, PROT_READ | PROT_WRITE, MAP_SHARED, zero_fd, 0);
	int *n_childs = mem;
	*n_childs     = 0;

	sem_t *plock = (sem_t *)mem + 1; // Accounted for in the offset
	sem_init(plock, 1, (unsigned)max_childs);

	int *processed = (int *)((sem_t *)mem + 2);
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
	free(tsorted);
}

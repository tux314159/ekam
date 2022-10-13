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
	bool _BFS_BEGIN_VIS[_BFS_BEGIN_QSZ];                          \
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
		.deps    = calloc_s(MAX_NODES, sizeof(struct Node)),
		.n_nodes = 0,
	};
}

void
graph_delete(struct Graph *g)
{
	for (int i = 0; i < MAX_NODES; i++) {
		free(g->deps[i].adj);
		free(g->deps[i].cmd);
		free(g->deps[i].filename);
	}
	free(g->deps);
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
	if (!(from && to)) {
	}

	struct Node *ato = g->deps + to;
	g->n_nodes += g->deps[from].len == 0;
	g->n_nodes += g->deps[to].len == 0;
	adjlist_add(ato, from);
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
	struct Node *afrom = g->deps + at;

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
		graph_add_edge(g, *d, target);
	}
	return;
}

static void
_bfs_copy(struct Graph *src, struct Graph *dest, size_t start, bool invert)
{
	BFS_BEGIN(queue, MAX_NODES, h, t, visited, start);

	while (h != t) {
		size_t c = QUEUE_POP(queue, h, MAX_NODES);
		graph_add_meta(dest, c, src->deps[c].cmd, src->deps[c].filename);

		for (size_t *n = src->deps[c].adj;
		     n < src->deps[c].adj + src->deps[c].len;
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
graph_buildpartial(
	struct Graph *src,
	struct Graph *dest,
	size_t       *starts,
	size_t        n_starts
)
{
	// Build inverted graph with all deps.
	struct Graph inv = graph_make();
	graph_add_meta(&inv, 0, "", "");
	for (size_t *i = starts; i < starts + n_starts; i++) {
		_bfs_copy(src, &inv, *i, true);
		graph_add_edge(&inv, *i, 0);
	}

	// BFS on source to find leaves.
	size_t *leaves = malloc_s(0);
	size_t  n_leaves = 0;
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, 0);
		while (h != t) {
			size_t c = QUEUE_POP(queue, h, MAX_NODES);
			if (src->deps[c].len == 0) { // no deps, i.e. leaf
				leaves = realloc_s(leaves, sizeof(size_t) * ++n_leaves);
				leaves[n_leaves - 1] = c;
			}

			for (size_t *n = src->deps[c].adj;
				 n < src->deps[c].adj + src->deps[c].len;
				 n++) {
				if (!visited[*n]) {
					visited[*n] = true;
					QUEUE_PUSH(queue, t, MAX_NODES, *n);
				}
			}
		}
	}

	// BFS and read mtimes.
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, 0);
		while (h != t) {
			size_t c = QUEUE_POP(queue, h, MAX_NODES);
			for (size_t *n = inv.deps[c].adj;
				 n < inv.deps[c].adj + inv.deps[c].len;
				 n++) {
				if (!visited[*n]) {
					visited[*n] = true;
					QUEUE_PUSH(queue, t, MAX_NODES, *n);
				}
			}
		}
	}

	free(leaves);
	return;
}

void
graph_execute(struct Graph *g, int max_childs)
{
	size_t tsorted[MAX_NODES];
	memset(tsorted, 0, sizeof(size_t) * MAX_NODES);

	BFS_BEGIN(queue, MAX_NODES, h, t, visited, 0);

	// Toposort
	for (size_t i = 0; h != t; i++) {
		size_t c = QUEUE_POP(queue, h, MAX_NODES);

		tsorted[i] = c;

		// Enqueue dependencies
		for (size_t *n = g->deps[c].adj; n < g->deps[c].adj + g->deps[c].len;
		     n++) {
			if (!visited[*n]) {
				visited[*n] = true;
				QUEUE_PUSH(queue, t, MAX_NODES, *n);
			}
		}
	}

	// Reverse toposorted array - it was actually done in reverse
	for (size_t i = 0; i < g->n_nodes / 2; i++) {
		size_t tmp                  = tsorted[i];
		tsorted[i]                  = tsorted[g->n_nodes - i - 1];
		tsorted[g->n_nodes - i - 1] = tmp;
	}

	// Set up shm
	// XXX: is this portable? Will sem_t ever be smaller than int?
	const size_t shm_sz = sizeof(int) + (sizeof(sem_t) - sizeof(int)) +
	                      sizeof(sem_t) + sizeof(int) * MAX_NODES;
	int   zero_fd = open("/dev/zero", O_RDWR);
	void *mem     = mmap(
        NULL,
        shm_sz,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        zero_fd,
        0
    ); // For clarity
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
		sem_wait(plock);

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
			for (size_t *d = g->deps[*c].adj;
			     d < g->deps[*c].adj + g->deps[*c].len;
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
					system(g->deps[*c].cmd);
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
}

/*
 * File stuff
 */

/*
 * Recursively find all the mtimes of all files/dirs
 * under the directory named rtname and store them in
 * the global hashtable.
 */
static void
store_mtimes(char *rtname)
{
	DIR           *root = opendir(rtname);
	struct dirent *child;
	while ((child = readdir(root))) {
		struct stat sbuf;
		stat(child->d_name, &sbuf);

		// Get mtime, insert in hashmap
		char *fullpath =
			malloc_s(strlen(rtname) + 1 + strlen(child->d_name) + 1);
		// lol
		strcpy(fullpath, rtname);
		fullpath[strlen(rtname)] = '/';
		strcpy(fullpath + strlen(rtname) + 1, child->d_name);
		fullpath[strlen(rtname) + 1 + strlen(child->d_name)] = '\0';

		ENTRY e = {.key = fullpath, .data = (void *)sbuf.st_mtime};
		if (!hsearch(e, ENTER)) {
			die(1, "Hashmap insertion failed!");
		}

		if (S_ISDIR(sbuf.st_mode)) {
			// Recurse if dir
		}
	}
}

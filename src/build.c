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

#include "build.h"
#include "graph.h"
#include "vector.h"

void
cons_partgraph(struct Graph *src, struct Graph *dest, size_t start)
{
	// BFS on source to find every vertex in direct need
	// of updating reachable from start.
	struct Vec_size_t needupd;
	VEC_INIT(needupd);
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, start);
		while (h != t) {
			size_t c = QUEUE_POP(queue, h, MAX_NODES);
			struct Node c_nde = src->nodes[c];

			// Read mtime
			struct stat csb;
			int csr = stat(c_nde.filename, &csb);

			// If we do not exist, we definitely need some updating.
			if (csr == -1) {
				VEC_PUSH(needupd, c);
			}

			if (!c_nde.adj) {
				continue;
			}

			for (size_t *n = c_nde.adj; n < c_nde.adj + c_nde.len; n++) {
				// We want every pair, so we do this before visited check;
				// at most we get one duplicate (I think) (I hope)
				struct stat nsb;
				int nsr = stat(src->nodes[*n].filename, &nsb);

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
	graph_copy(src, &inv, start, true);

	// Do multisource BFS with nodes needing update as starting points
	// and therefore construct dest.
	{
		BFS_BEGIN(queue, MAX_NODES, h, t, visited, 0);

		memcpy(queue, needupd.arr, needupd.sz * sizeof(*needupd.arr));
		for (size_t i = 0; i < needupd.sz; i++)
			visited[needupd.arr[i]] = true;
		t = needupd.sz;

		while (h != t) {
			size_t c = QUEUE_POP(queue, h, MAX_NODES);
			struct Node c_nde = inv.nodes[c];
			// Copy metadata over
			graph_add_meta(dest, c, src->nodes[c].cmd, src->nodes[c].filename);

			if (!c_nde.adj) {
				continue;
			}

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

void
build_graph(struct Graph *g, int max_childs)
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
		size_t tmp = tsorted[i];
		tsorted[i] = tsorted[g->n_nodes - i - 1];
		tsorted[g->n_nodes - i - 1] = tmp;
	}

	// Time to finally start executing shit!

	// Set up shm
	const size_t shm_sz = sizeof(sem_t) + sizeof(int) + sizeof(int) * MAX_NODES;
	int zero_fd = open("/dev/zero", O_RDWR);
	void *mem =
		mmap(NULL, shm_sz, PROT_READ | PROT_WRITE, MAP_SHARED, zero_fd, 0);

	sem_t *plock = mem;
	sem_init(plock, 1, (unsigned)max_childs);

	int *n_childs = (int *)((sem_t *)mem + 1);
	*n_childs = 0;

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
			if (g->nodes[*c].adj) {
				for (size_t *d = g->nodes[*c].adj;
				     d < g->nodes[*c].adj + g->nodes[*c].len;
				     d++) {
					deps_sat &= (processed[*d] == 2);
				}
			}

			if (deps_sat) {
				(*n_childs)++;
				processed[*c] = 1;
				cnt++;
				if (g->nodes[*c].cmd && g->nodes[*c].cmd[0]) {
					// We don't wanna spam blank lines :p
					printf("%s\n", g->nodes[*c].cmd);
				}
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

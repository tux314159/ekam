#include "ekam.h"
#include "graph.h"
#include "hashtable.h"
#include "safealloc.h"
#include "util/debug.h"
#include <stdarg.h>
#include <stdio.h>

void
adddeps_resolve(
	struct Graph     *graph,
	Hashtable_size_t *ht,
	const char       *filename,
	const char       *cmd,
	size_t            n_deps,
	...
)
{
	va_list deps_va;
	va_start(deps_va, n_deps);

	size_t *deps = malloc_s(n_deps * sizeof(*deps));
	for (size_t i = 0; i < n_deps; i++) {
		char   *dep    = va_arg(deps_va, char *);
		size_t *depptr = ht_get_size_t(ht, dep);
		if (depptr) {
			deps[i] = *depptr;
		} else {
			die(1, "FATAL: %s: no such target!\n", dep);
		}
	}

	size_t *targptr = ht_get_size_t(ht, filename);
	if (targptr) {
		graph_add_target(graph, *targptr, deps, n_deps, cmd, filename);
	} else {
		die(1, "FATAL: %s: no such target!\n", filename);
	}

	free_s(deps);
	va_end(deps_va);
}

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ekam.h"

#define DECLARE_C(base)              \
	DECLARE_("src/" #base ".c");     \
	DECLARE_("include/" #base ".h"); \
	ID_("src/" #base ".c");          \
	ID_("include/" #base ".h");      \
	DECLARE_("build/" #base ".o");

// clang-format off
#define CFLAGS       -Iinclude
#define CC(in, out)  gcc CFLAGS -o out in
#define OCC(in, out) gcc CFLAGS -c -o out in
// clang-format on
#define ID_(in)     D0_(in, "")
#define ID(in)      ID_(Q(in))

int
main(int argc, char **argv)
{
	INIT_EKAM();

	// clang-format off
	DECLARE_C(graph);
	DECLARE_C(safealloc);
	DECLARE(include/ekam.h);
	DECLARE(main.c);
	DECLARE(build/main);

	ID(include/ekam.h);
	ID(main.c);

	D(build/graph.o,
		OCC(build/graph.c, src/graph.o),
		R(src/graph.c), R(include/graph.h));
	D(build/safealloc.o,
		OCC(build/safealloc.c, src/safealloc.o),
		R(src/safealloc.c), R(include/safealloc.h));
	D(build/main,
		CC(main.c, build/main),
		R(main.c), R(include/ekam.h), R(build/graph.o), R(build/safealloc.o));
	// clang-format on

	BUILD_TARGET(build/main);
	FREE_EKAM();
}

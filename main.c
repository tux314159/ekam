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
#define CFLAGS -Iinclude -O2

#define CC_(out) "gcc " Q(CFLAGS) " -o " out " "
#define CC(out) CC_(#out)

#define OCC_(out) CC_(out) " -c "
#define OCC(out) OCC_(#out)

#define ID_(in) D0_(in, "")
#define ID(in) ID_(#in)

#define BUILD_COBJ(base)								     \
	D_("build/" #base ".o",                                  \
		OCC_("build/" #base ".o") "src/" #base ".c", \
		R_("src/" #base ".c"), R_("include/" #base ".h"))

int
main(int argc, char **argv)
{
	INIT_EKAM();

	DECLARE_C(safealloc);
	DECLARE_C(graph);
	DECLARE_C(build);
	DECLARE(include/ekam.h);
	DECLARE(main.c);
	DECLARE(build/main);
	DECLARE(build);

	ID(include/ekam.h);
	ID(main.c);

	BUILD_COBJ(build);
	BUILD_COBJ(graph);
	BUILD_COBJ(safealloc);

	D_("build/main",
		CC(build/main) "main.c build/graph.o build/safealloc.o "
			"build/build.o",
		R(main.c), R(include/ekam.h), R(build/graph.o),
		R(build/safealloc.o), R(build/build.o));

	system("mkdir -p build");
	BUILD_TARGET(build/main);
	FREE_EKAM();
}

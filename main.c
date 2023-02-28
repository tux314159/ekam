#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ekam.h"

#define DECLARE_COBJ(base)           \
	DECLARE_("src/" #base ".c");     \
	DECLARE_("include/" #base ".h"); \
	ID_("src/" #base ".c");          \
	ID_("include/" #base ".h");      \
	DECLARE_("build/" #base ".o");

// clang-format off
#define BUILDDIR build
#define SRCDIR src
#define HEADERDIR include
#define CFLAGS -I HEADERDIR -O2

#define CC_(out) "gcc " Q(CFLAGS) " -o " out " "
#define CC(out) CC_(#out)

#define OCC_(out) CC_(out) " -c "
#define OCC(out) OCC_(#out)

#define ID_(in) D0_(in, "")
#define ID(in) ID_(#in)

#define BUILD_COBJ(base)                                       \
	D_(Q(BUILDDIR) "/" #base ".o",                             \
		OCC_(Q(BUILDDIR) "/" #base ".o") Q(SRCDIR) #base ".c", \
		R_(Q(SRCDIR) "/" #base ".c"), R_(Q(HEADERDIR) "/" #base ".h"))

#define DECLARE_ID_(in) do { DECLARE_(in); ID_(in); } while (0)
#define DECLARE_ID(in) DECLARE_ID_(#in)

int
main(int argc, char **argv)
{
	INIT_EKAM();

	DECLARE_COBJ(safealloc);
	DECLARE_COBJ(graph);
	DECLARE_COBJ(build);
	DECLARE_ID(include/ekam.h);
	DECLARE_ID(main.c);
	DECLARE(build/main);
	DECLARE(build);

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

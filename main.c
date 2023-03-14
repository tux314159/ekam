#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ekam.h"

// clang-format off
#define BUILDDIR "build"
#define SRCDIR "src"
#define HEADERDIR "include"
#define CC "gcc"
#define CFLAGS " -O2"

#include "builders/basic.h"
#include "builders/c.h"

int
main(int argc, char **argv)
{
	INIT_EKAM();

	DECLARE_COBJ("safealloc");
	DECLARE_COBJ("graph");
	DECLARE_COBJ("build");
	DECLARE_ID("include/ekam.h");
	DECLARE_ID("main.c");
	DECLARE("build/main");

	BUILD_COBJ("build");
	BUILD_COBJ("graph");
	BUILD_COBJ("safealloc");

	D("build/main",
		BUILD_C("build/main") "main.c build/graph.o build/safealloc.o "
			"build/build.o",
		R("main.c"), R("include/ekam.h"), R("build/graph.o"),
		R("build/safealloc.o"), R("build/build.o"));

	system("mkdir -p build");
	BUILD_TARGET("build/main");
	FREE_EKAM();
}

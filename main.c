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
	DECLARE_COBJ("ekam");
	DECLARE_ID("main.c");
	DECLARE("build/main");

	BUILD_COBJ("safealloc");
	BUILD_COBJ("build");
	BUILD_COBJ("graph");
	BUILD_COBJ("ekam");

	D("build/main",
		BUILD_C("build/main") "main.c build/graph.o build/safealloc.o build/build.o build/ekam.o",
		"main.c", DEP_COBJ("graph"), DEP_COBJ("safealloc"), DEP_COBJ("build"), DEP_COBJ("ekam"));

	system("mkdir -p build");
	BUILD_TARGET("build/main");
	FREE_EKAM();
}

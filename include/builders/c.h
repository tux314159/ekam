// For building C targets.

#ifndef INCLUDE_BUILDERS_C
#define INCLUDE_BUILDERS_C

#include "basic.h"

#ifndef HEADERDIR
#error HEADERDIR not set!
#endif
#ifndef CC
#error CC not set!
#endif
#ifndef CFLAGS
#error CFLAGS not set!
#endif

#include "ekam.h"

// Compiler macros.
#define BUILD_C(out) CC " -I" HEADERDIR " " CFLAGS " -o " out " "
#define BUILD_C_O(out) BUILD_C(out) " -c "

// A "C object" consists of a source file and a header file, which
// produce an object file. All three should have the same basename.
// The source, header, and object files should reside in SRCDIR,
// HEADERDIR, and BUILDDIR respectively.

#define DECLARE_COBJ(base)         \
	DECLARE("src/" base ".c");     \
	DECLARE("include/" base ".h"); \
	ID("src/" base ".c");          \
	ID("include/" base ".h");      \
	DECLARE("build/" base ".o");

#define BUILD_COBJ(base)                                           \
	D(BUILDDIR "/" base ".o",                                      \
		BUILD_C_O(BUILDDIR "/" base ".o") SRCDIR "/" base ".c",    \
		R(SRCDIR "/" base ".c"), R(HEADERDIR "/" base ".h"))

#endif

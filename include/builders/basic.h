// Every builder should include us!
// Has some basic builder macros.

#ifndef INCLUDE_BUILDERS_BASIC
#define INCLUDE_BUILDERS_BASIC

#ifndef SRCDIR
#error SRCDIR not set!
#endif
#ifndef BUILDDIR
#error BUILDDIR not set!
#endif

#include "ekam.h"

// This one does nothing.
#define ID(in) D0(in, "")

#define DECLARE_ID(in) \
	do {               \
		DECLARE(in);   \
		ID(in);        \
	} while (0)

#endif

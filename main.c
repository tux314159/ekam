#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"
#include "hashtable.h"
#include "ekam.h"

int
main(int argc, char **argv)
{
	INIT_EKAM();

	DECLARE(test/1)
	DECLARE(test/2)
	DECLARE(test/3)
	DECLARE(test/4)
	DECLARE(test/5)
	DECLARE(test/6)
	DECLARE(test/7)
	DECLARE(test/8)
	DECLARE(test/9)
	DECLARE(test/10)

	// clang-format off
	_(test/10,
		echo at 10; sleep 1.0; touch test/10,
		R(test/2), R(test/3)
	);
	_(test/1,
		echo at 1; sleep 1.0; touch test/1,
		R(test/3), R(test/4)
	);
	_(test/2,
		echo at 2; sleep 1.0; touch test/2,
		R(test/5)
	);
	__(test/3,
			echo at 3; sleep 1.0; touch test/3
	);
	_(test/4,
		echo at 4; sleep 1.0; touch test/4,
		R(test/5)
	);
	__(test/5,
		echo at 5; sleep 1.0; touch test/5
	);
	_(test/6,
		echo at 6; sleep 1.0; touch test/6,
		R(test/2), R(test/8), R(test/9)
	);
	_(test/7,
		echo at 7; sleep 1.0; touch test/7,
		R(test/1), R(test/10), R(test/6)
	);
	_(test/8,
		echo at 8; sleep 1.0; touch test/8,
		R(test/3)
	);
	_(test/9,
		echo at 9; sleep 1.0; touch test/9,
		R(test/3)
	);
	// clang-format on

	BUILD_TARGET(test/7);
	FREE_EKAM();
}

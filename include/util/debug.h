/*
 *   Copyright (C) 2021  Isaac "Tux" Yeo
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

#define NULLDIE(x)                       \
	do {                                 \
		if (x == NULL) {                 \
			fprintf(                     \
				stderr,                  \
				"NULL @ %s in %s:%d!\n", \
				__func__,                \
				__FILE__,                \
				__LINE__                 \
			);                           \
			exit(1);                     \
		};                               \
	} while (0);

#ifndef NDEBUG

#define debugpf(...)                                                           \
	do {                                                                       \
		fprintf(stderr, __VA_ARGS__);                                          \
		fprintf(stderr, "    [@%s in %s:%d]\n", __func__, __FILE__, __LINE__); \
	} while (0);
#define dbgidntpf(level, ...)                      \
	do {                                           \
		for (size_t ___ = 0; ___ < level; ___++) { \
			fprintf(stderr, " ");                  \
		}                                          \
		debugpf(__VA_ARGS__);                      \
	} while (0);

#else

#define debugpf(...)          ;
#define dbgidntpf(level, ...) ;

#endif

#define die(status, ...)      \
	do {                      \
		debugpf(__VA_ARGS__); \
		exit(status);         \
	} while (0)

#endif

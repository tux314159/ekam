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

#ifndef RULE_H
#define RULE_H

#include <stdbool.h>
#include <stddef.h>

// basically a graph lol
struct rule_t {
    int             id;       // sequential!
    char ***        commands; // array of arrays of strings for use with execvp
    struct rule_t **rdeps;    // array of things depending on us
    size_t          rdeps_n;  // no. of reverse deps
    bool            satisfiedp; // satisfied?
    bool            headp;      // main target?
};
#endif

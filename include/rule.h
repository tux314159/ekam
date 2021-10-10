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

struct rule_t {
    size_t id;
    char **commands;   // array of commands, for use with execlp (sh -c for now)
    size_t commands_n; // number of commands
    bool   satisfiedp; // satisfied?
};
#endif

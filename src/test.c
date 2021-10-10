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

#include <debug.h>
#include <graph.h>
#include <stdio.h>

int main(void)
{
    struct adjlist_t *al;
    struct adjlist_t *partial;
    size_t *          t;

    al      = calloc(1, sizeof(*al));
    partial = calloc(1, sizeof(*partial));
    t       = calloc(3, sizeof(*t));
    NULLDIE(al);
    NULLDIE(partial);
    NULLDIE(t);
    t[0] = 1;
    t[1] = 3;
    t[2] = 4;

    for (size_t i = 0; i < 9; i++) {
        struct rule_t *r = calloc(1, sizeof(*r));
        NULLDIE(r);
        r->commands = calloc(1, sizeof(*r->commands));
        NULLDIE(r->commands);
        r->commands[0] = calloc(100, sizeof(**r->commands));
        NULLDIE(r->commands[0]);
        sprintf(r->commands[0], "echo NUMBER %lu", i + 1);
        r->commands_n = 1;
        r->id         = i;
        r->satisfiedp = false;
        g_add_node(al, i, r);
    }

    g_add_edge(al, 0, 5);
    g_add_edge(al, 1, 5);
    g_add_edge(al, 1, 6);
    g_add_edge(al, 2, 6);
    g_add_edge(al, 3, 7);
    g_add_edge(al, 4, 7);
    g_add_edge(al, 5, 8);
    g_add_edge(al, 6, 8);
    g_add_edge(al, 7, 8);

    g_construct_partial(al, partial, 3, t);
    g_exec_graph(partial);

    return 0;
}

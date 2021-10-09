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

#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <stddef.h>

#include "rule.h"
#include "uthash.h"

// DAG!

struct node_t {
    size_t         id;      // unique
    size_t *       adj;     // must be malloc'd
    size_t         conn_n;  // out-degree
    struct rule_t  data;
    UT_hash_handle hh;
};

struct adjlist_t {
    size_t         V;
    size_t         E;
    struct node_t *adjlist;  // maps nodeid <-> node
};

/*
 * Given the main target and updated files, construct
 * the partial graph.
 *
 * all fields of part_adjlist must be zeroed.
 *
 * part_adjlist->adjlist will be filled with new allocations;
 * remember to free those!
 */
void g_construct_partial(
    const struct adjlist_t *full_adjlist,
    struct adjlist_t *      part_adjlist,
    size_t                  updated_n,
    const size_t *          updated);

/*
 * Adds an empty to a graph.
 * Assumes id is unique!
 */
void g_add_node(struct adjlist_t *adjlist, size_t id);

/*
 * Adds an edge from from to to.
 */
void g_add_edge(struct adjlist_t *adjlist, size_t from, size_t to);

#endif

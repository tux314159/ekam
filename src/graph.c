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
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uthash.h>

struct _visited_t {
    size_t         id;
    bool           visitedp; // doesn't actually matter lol
    UT_hash_handle hh;
};

static void _construct_partial(
    const struct adjlist_t *full_adjlist,
    struct adjlist_t *      part_adjlist,
    struct _visited_t **    visited,
    const size_t            node
#ifndef NDEBUG
    ,
    size_t lvl
#endif
) {
    // DFS

    struct _visited_t *node_visited_p, *new_visited_p;
    struct node_t *    nodedata, *node_new;

    HASH_FIND_SIZET(*visited, &node, node_visited_p);

    if (node_visited_p) {
        dbgidntpf(lvl, "node %lu done - skipping\n", node);
        return;
    }

    dbgidntpf(lvl, "processing node %lu\n", node);

    NULLDIE((new_visited_p = malloc(sizeof(*new_visited_p))));
    new_visited_p->id       = node;
    new_visited_p->visitedp = true;
    HASH_ADD_SIZET(*visited, id, new_visited_p);

    HASH_FIND_SIZET(full_adjlist->adjlist, &node, nodedata);
    NULLDIE(nodedata);

    // add current node to partial graph
    NULLDIE((node_new = malloc(sizeof(*node_new))));
    memcpy(node_new, nodedata, sizeof(*node_new));
    NULLDIE(
        (node_new->adj = malloc(sizeof(*node_new->adj) * node_new->conn_n)));
    memcpy(
        node_new->adj,
        nodedata->adj,
        sizeof(*node_new->adj) * node_new->conn_n);
    HASH_ADD_SIZET(part_adjlist->adjlist, id, node_new);

    part_adjlist->V += 1;
    for (size_t *c = nodedata->adj; c < nodedata->adj + nodedata->conn_n; c++) {
        dbgidntpf(lvl, "recursing into node %lu\n", *c);
        part_adjlist->E += 1;
#ifndef NDEBUG
        _construct_partial(full_adjlist, part_adjlist, visited, *c, lvl + 1);
#else
        _construct_partial(full_adjlist, part_adjlist, visited, *c);
#endif
    }

    dbgidntpf(lvl, "returning from node %lu\n", node);
    return;
}

void g_construct_partial(
    const struct adjlist_t *full_adjlist,
    struct adjlist_t *      part_adjlist,
    size_t                  updated_n,
    const size_t *          updated) {
    struct _visited_t *visited;
    struct _visited_t *i, *tmp;

    visited = NULL;

    for (const size_t *c = updated; c < updated + updated_n; c++) {
#ifndef NDEBUG
        _construct_partial(full_adjlist, part_adjlist, &visited, *c, 0);
#else
        _construct_partial(full_adjlist, part_adjlist, &visited, *c);
#endif
    }

    HASH_ITER(hh, visited, i, tmp) {
        HASH_DEL(visited, i);
        free(i);
    }

    return;
}

void g_add_node(struct adjlist_t *adjlist, size_t id) {
    struct node_t *node;

    NULLDIE((node = malloc(sizeof(*node))));
    node->adj    = malloc(0);
    node->conn_n = 0;
    node->id     = id;

    adjlist->V += 1;
    HASH_ADD_SIZET(adjlist->adjlist, id, node);

    return;
}

void g_add_edge(struct adjlist_t *adjlist, size_t from, size_t to) {
    struct node_t *fromnode;
    size_t *       t;

    HASH_FIND_SIZET(adjlist->adjlist, &from, fromnode);
    NULLDIE(fromnode);
    t = realloc(fromnode->adj, sizeof(*fromnode->adj) * (fromnode->conn_n + 1));
    fromnode->adj                   = t;
    fromnode->adj[fromnode->conn_n] = to;

    fromnode->conn_n += 1;
    adjlist->E += 1;

    return;
}

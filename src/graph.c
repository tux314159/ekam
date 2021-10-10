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
#include <sys/wait.h>
#include <unistd.h>
#include <uthash.h>

struct _visited_t {
    size_t         id;
    UT_hash_handle hh;
};

// Function to iterate over a graph and also generate graph of visited nodes
static void _iter_graph(
    const struct adjlist_t *full_adjlist,
    struct adjlist_t *      part_adjlist,
    struct _visited_t **    visited,
    const size_t            node,
    void (*fn)(struct node_t *)
#ifndef NDEBUG
        ,
    size_t lvl
#endif
)
{
    // DFS

    struct _visited_t *node_visited_p, *new_visited_p;
    struct node_t *    nodedata, *node_new;

    HASH_FIND_SIZET(*visited, &node, node_visited_p);

    if (node_visited_p) {
        dbgidntpf(lvl, "node %lu done - skipping", node);
        return;
    }

    dbgidntpf(lvl, "processing node %lu", node);

    NULLDIE((new_visited_p = malloc(sizeof(*new_visited_p))));
    new_visited_p->id = node;
    HASH_ADD_SIZET(*visited, id, new_visited_p);

    HASH_FIND_SIZET(full_adjlist->adjlist, &node, nodedata);
    NULLDIE(nodedata);

    // run the function _before_ doing anything with the node
    fn(nodedata);

    if (part_adjlist != NULL) {
        // add current node to partial graph
        NULLDIE((node_new = malloc(sizeof(*node_new))));
        memcpy(node_new, nodedata, sizeof(*node_new));
        NULLDIE((
            node_new->adj = malloc(sizeof(*node_new->adj) * node_new->conn_n)));
        memcpy(
            node_new->adj,
            nodedata->adj,
            sizeof(*node_new->adj) * node_new->conn_n);
        HASH_ADD_SIZET(part_adjlist->adjlist, id, node_new);

        part_adjlist->V += 1;
    }

    // recursive fun!
    for (size_t *c = nodedata->adj; c < nodedata->adj + nodedata->conn_n; c++) {
        dbgidntpf(lvl, "recursing into node %lu", *c);
        if (part_adjlist != NULL) {
            part_adjlist->E += 1;
        }
#ifndef NDEBUG
        _iter_graph(full_adjlist, part_adjlist, visited, *c, fn, lvl + 1);
#else
        _iter_graph(full_adjlist, part_adjlist, visited, *c, fn);
#endif
    }

    dbgidntpf(lvl, "returning from node %lu", node);
    return;
}

// we just want to construct partial
static void _dummy(struct node_t *in)
{
    (void)in;
    return;
}

void g_construct_partial(
    const struct adjlist_t *full_adjlist,
    struct adjlist_t *      part_adjlist,
    size_t                  updated_n,
    const size_t *          updated)
{
    struct _visited_t *visited;
    struct _visited_t *i, *tmp;

    visited = NULL;

    for (const size_t *c = updated; c < updated + updated_n; c++) {
#ifndef NDEBUG
        _iter_graph(full_adjlist, part_adjlist, &visited, *c, &_dummy, 0);
#else
        _iter_graph(full_adjlist, part_adjlist, &visited, *c, &_dummy);
#endif
    }

    // increment all ids in partial graph by 1 -- by creating a new one loll
    {
        struct node_t *c, *tmp2;
        struct node_t *newadj;

        newadj = NULL;
        HASH_ITER(hh, part_adjlist->adjlist, c, tmp2)
        {
            struct node_t *new;

            new = malloc(sizeof(*new));
            NULLDIE(new);
            memcpy(new, c, sizeof(*c));
            for (size_t *p = new->adj; p < new->adj + new->conn_n; p++) {
                *p += 1;
            }
            new->id += 1;

            HASH_DEL(part_adjlist->adjlist, c);
            free(c); // NOTE: do not free substructures; still in use by new!

            HASH_ADD_SIZET(newadj, id, new);
        }

        part_adjlist->adjlist = newadj;
    }
    // insert dummy node 0, for easier tracking
    g_add_node(part_adjlist, 0, NULL);
    for (const size_t *c = updated; c < updated + updated_n; c++) {
        g_add_edge(part_adjlist, 0, *c + 1);
    }

    HASH_ITER(hh, visited, i, tmp)
    {
        HASH_DEL(visited, i);
        free(i);
    }

    return;
}

static void _exec_node(struct node_t *node)
{
    if (node->data == NULL || node->data->commands == NULL) {
        return;
    }

    for (char **c = node->data->commands;
         c < node->data->commands + node->data->commands_n;
         c++) {
        system(*c);
    }
    return;
}

void g_exec_graph(const struct adjlist_t *adjlist)
{
    struct _visited_t *visited;
    struct _visited_t *i, *tmp;

    visited = NULL;

#ifndef NDEBUG
    _iter_graph(adjlist, NULL, &visited, 0, &_exec_node, 0);
#else
    _iter_graph(adjlist, NULL, &visited, 0, &_exec_node);
#endif

    HASH_ITER(hh, visited, i, tmp)
    {
        HASH_DEL(visited, i);
        free(i);
    }

    return;
}

void g_add_node(struct adjlist_t *adjlist, size_t id, struct rule_t *data)
{
    struct node_t *node;

    NULLDIE((node = malloc(sizeof(*node))));
    node->id     = id;
    node->adj    = malloc(0);
    node->conn_n = 0;
    node->data   = malloc(sizeof(*node->data));
    NULLDIE(node->data)
    if (data != NULL) {
        memcpy(node->data, data, sizeof(*data));
    } else {
        free(node->data);
        node->data = NULL;
    }

    adjlist->V += 1;
    HASH_ADD_SIZET(adjlist->adjlist, id, node);

    return;
}

void g_add_edge(struct adjlist_t *adjlist, size_t from, size_t to)
{
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

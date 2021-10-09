#ifndef GRAPH_H
#define GRAPH_H

#include "rule.h"
#include "uthash.h"

#include <stdbool.h>
#include <stddef.h>

// DAG!

struct node_t {
    size_t          id;         // unique
    size_t          *adj;       // must be malloc'd
    size_t          conn_n;     // out-degree
    struct rule_t   data;
    UT_hash_handle  hh;
};

struct adjlist_t {
    size_t          V;
    size_t          E;
    struct node_t   *adjlist;   // maps nodeid <-> node
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
        const struct adjlist_t  *full_adjlist,
        struct adjlist_t        *part_adjlist,
        size_t                  updated_n,
        const size_t            *updated
);

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

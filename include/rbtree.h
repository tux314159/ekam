#ifndef INCLUDE_RBTREE_H
#define INCLUDE_RBTREE_H

typedef struct Redblack {
    struct Redblack *l, *r, *parent;
    long val;
    char col; // 1 is red
} Redblack;

void rb_insert(Redblack *tree, long val);

#endif

#include "rbtree.h"
#include <stdlib.h>

static void rb_balance(Redblack *inserted)
{
	// We are root.
	if (!inserted->parent) {
		inserted->col = 0;
		return;
	}

	// Red-black; nothing to do.
	if (!inserted->parent->col) {
		return;
	}

	// Note that there will always be a grandparent,
	// else red-black would've been satisfied.
	Redblack *parent, *uncle;
	Redblack **gppptr; // grandparent-parent-pointer
	parent = inserted->parent;
	if (parent == parent->parent->l) {
		uncle = parent->parent->r;
		gppptr = &parent->parent->l;
	} else {
		uncle = parent->parent->l;
		gppptr = &parent->parent->r;
	}
	char unclecol = uncle ? uncle->col : 0;

	if (inserted->parent->col && unclecol) {
		// Parent and uncle are red.
		parent->parent->col = 1;
		parent->col = 0;
		if (uncle) {
			uncle->col = 0;
		}
		rb_balance(parent->parent);
	} else if (parent->col && !unclecol && parent->r == inserted) {
		// Parent red, uncle black, we are right child.
		// Left-rotation around parent.
		*gppptr = inserted;
		inserted->parent = parent->parent;

		parent->r = inserted->l;
		if (inserted->l) {
			parent->l = inserted->l;
			inserted->l->parent = parent;
		} else {
			parent->l = NULL;
		}

		inserted->l = parent;
		parent->parent = inserted;
	} else {
		// Parent red, grandparent black, we are left child.
		// Right-rotation around grandparent.
	}
	
}

void rb_insert(Redblack *tree, long val)
{
	Redblack *inschild;
	if (val > tree->val) {
		inschild = tree->r;
	} else {
		inschild = tree->l;
	}

	if (inschild) {
		rb_insert(inschild, val);
	} else {
		Redblack *nde = calloc(1, sizeof(*nde));
		nde->val = val;
		nde->col = 1;
		nde->parent = inschild;
		inschild = nde;
	}
}

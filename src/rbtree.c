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

	// Parent and uncle are red.
	if (inserted->parent->col && unclecol) {
		parent->parent->col = 1;
		parent->col = 0;
		if (uncle) {
			uncle->col = 0;
		}
		rb_balance(parent->parent);
	}
	// Parent red, uncle black, we are right child.
	if (parent->col && !unclecol && parent->r == inserted) {
		// Left-rotation.
		*gppptr = inserted;
		inserted->parent = parent->parent;

		if (inserted->l) {
			inserted->l->parent = parent;
		}
		parent->r = inserted->l;

		parent->parent = inserted;
		inserted->l = parent;
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

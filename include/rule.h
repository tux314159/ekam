#ifndef RULE_H
#define RULE_H

#include <stdbool.h>
#include <stddef.h>

// basically a graph lol
struct rule_t {
    int             id;             // sequential!
    char            ***commands;    // array of arrays of strings for use with execvp
    struct rule_t   **rdeps;        // array of things depending on us
    size_t          rdeps_n;        // no. of reverse deps
    bool            satisfiedp;     // satisfied?
    bool            headp;          // main target?
};
#endif

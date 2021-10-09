#! /bin/sh
echo "Temporary build script-- will be replaced once we can build ourselves."
gcc -std=gnu99 -Iinclude -pedantic -Wall -Wextra -Wpedantic -Wbad-function-cast -Wcast-align -Wcast-qual -Wdeclaration-after-statement -Wfloat-equal -Wformat=2 -Wmissing-declarations -Wmissing-include-dirs -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wsequence-point -Wshadow -Wsign-conversion -Wstrict-prototypes -Wswitch -Wundef -Wunreachable-code -Wunused-parameter -Wwrite-strings -Werror -o test src/test.c src/graph.c
echo "Test built."

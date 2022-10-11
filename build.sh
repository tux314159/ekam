#! /bin/sh

cflags=$(cat compile_flags.txt | tr '\n' ' ')

[ -z "$CC" ] && CC=gcc

echo "Building main"
$CC $cflags -o main main.c src/*.c -lpthread -lrt -lmd -lgdbm -lgdbm_compat

fs.c
graph.c
safealloc.c

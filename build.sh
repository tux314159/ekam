#! /bin/sh

cflags=$(cat compile_flags.txt | tr '\n' ' ')

[ -z "$CC" ] && CC=gcc

echo "Building main"
$CC $cflags -o main main.c src/*.c -lpthread -lrt

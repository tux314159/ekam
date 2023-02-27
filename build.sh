#! /bin/sh

cflags=$(cat compile_flags.txt | tr '\n' ' ')

[ -z "$CC" ] && CC=gcc

echo "Building main"
$CC $cflags -o _main -O2 $1 main.c src/*.c -lpthread -lrt
echo "Rebuilding main with ourselves"
./_main 8
rm _main

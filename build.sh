#! /bin/sh
echo "Temporary build script - will be replaced once we can build ourselves."
cflags=$(cat compile_flags.txt | tr '\n' ' ')
gcc $cflags -o main $@ src/graph.c src/safealloc.c src/main.c && echo "Built."

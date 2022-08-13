#! /bin/sh
echo "Generating amalgamation"
cpp -P -Iinclude everything.gen > everything
echo "Building main"
gcc -o -lpthread -lrt main main.c

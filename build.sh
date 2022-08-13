#! /bin/sh
echo "Generating amalgamation"
cpp -P -Iinclude ekam.gen > ekam
echo "Building main"
gcc -o main -lpthread -lrt main.c

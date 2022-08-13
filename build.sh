#! /bin/sh
echo "Generating amalgamation"
cpp -Iinclude everything.gen > everything

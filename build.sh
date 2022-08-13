#! /bin/sh
echo "Generating amalgamation"
cpp -P -Iinclude everything.gen > everything

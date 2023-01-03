#!/bin/sh -e

print() {
    echo $1 | (command -v boxes > /dev/null && boxes -d stone || cat)
}

if [ -n "$SKIP_BUILD" ]; then
    exit 0
fi

print "Building"
cmake -S . -B build
cmake --build build

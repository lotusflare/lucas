#!/bin/sh -e

print() {
    echo $1 | (command -v boxes > /dev/null && boxes -d stone || cat)
}

print "Building"
cmake -S . -B build
cmake --build build

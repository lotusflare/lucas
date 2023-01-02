#!/bin/sh -e

if [ -n "$SKIP_BUILD" ]; then
    exit 0
fi

cmake -S . -B build
cmake --build build

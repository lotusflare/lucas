#!/bin/sh -e

fix=false

parse_flags() {
    while getopts 'f' option; do
        case $option in
            f) fix=true;;
            ?) exit 1;;
        esac
    done
}

fix() {
    find src include -name '*.h' -o -name '*.c' | xargs clang-format --verbose -i
    find integration -name '*.lua' | xargs prettier -w
}

check() {
    find src include -name '*.h' -o -name '*.c' | xargs clang-format --verbose -n
    find integration -name '*.lua' | xargs prettier -c
}

parse_flags "$@"

$fix && fix || check

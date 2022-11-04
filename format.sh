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

format() {
    if $fix; then
        pargs='-w'
        cargs='-i'
    else
        pargs='-c'
        cargs='-n'
    fi
    find src include -name '*.h' -o -name '*.c' | xargs clang-format --verbose $cargs
    find integration -name '*.lua' | xargs prettier $pargs
}

parse_flags "$@"
format

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
        prettier_args='-w'
        clang_args='-i'
    else
        prettier_args='-c'
        clang_args='-n'
    fi
    find src include -name '*.h' -o -name '*.c' | xargs clang-format --verbose "$clang_args"
    find integration -name '*.lua' | xargs prettier "$prettier_args"
}

parse_flags "$@"
format

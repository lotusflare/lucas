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
    find src include -iname *.h -o -iname *.c | xargs clang-format --verbose -i
    find integration -iname *.lua | xargs prettier -w
}

check() {
    find src include -iname *.h -o -iname *.c | xargs clang-format --verbose -n
    find integration -iname *.lua | xargs prettier -c
}

parse_flags "$@"

$fix && fix || check

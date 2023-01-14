#!/bin/sh -e

source ./print.sh

fix=false

format_flags() {
    while getopts 'f' option; do
        case $option in
        f) fix=true ;;
        ?) exit 1 ;;
        esac
    done
}

format() {
    if $fix; then
        pargs='--verbose'
        cargs='-i'
    else
        pargs='--output-format summary -c'
        cargs='-n'
    fi
    print "Running clang-format"
    find src include -name '*.h' -o -name '*.c' | xargs clang-format --verbose $cargs
    print "Running stylua"
    find integration -name '*.lua' | xargs stylua $pargs
}

format_flags "$@"
format

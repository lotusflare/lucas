#!/bin/sh -e

. ./print.sh

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
        lua_args='-v'
        clang_args='--verbose -i'
    else
        lua_args='-c --output-format summary'
        clang_args='--verbose -n'
    fi

    print "Running clang-format"
    find src include -name '*.h' -o -name '*.c' | xargs clang-format $clang_args

    print "Running stylua"
    find integration -name '*.lua' | xargs stylua $lua_args
}

format_flags "$@"
format

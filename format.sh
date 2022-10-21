#!/bin/sh -e

fix=false

parse_flags() {
    while getopts 'f' OPTION; do
        case $OPTION in
            f) fix=true;;
            ?) exit 1;;
        esac
    done
}

fix() {
    find src include -iname *.h -o -iname *.c | xargs clang-format -i
    find integration -iname *.lua | xargs prettier -w
}

check() {
    find src include -iname *.h -o -iname *.c | xargs clang-format -n
    find integration -iname *.lua | xargs prettier --loglevel=silent -c
}

parse_flags

if $fix; then
    fix
else
    check
fi

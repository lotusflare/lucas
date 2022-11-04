#!/bin/sh -e

fix=false

format_flags() {
    while getopts 'f' option; do
        case $option in
        f) fix=true ;;
        ?) exit 1 ;;
        esac
    done
}

print_box() {
    length=$((${#1} + 1))
    printf "┏"
    for i in $(seq 0 $length); do
        printf "━"
    done
    printf "┓\n┃ %s ┃\n┗" "$1"
    for i in $(seq 0 $length); do
        printf "━"
    done
    echo "┛"
}

format() {
    if $fix; then
        pargs='-w'
        cargs='-i'
    else
        pargs='-c'
        cargs='-n'
    fi
    print_box "Running clang-format"
    find src include -name '*.h' -o -name '*.c' | xargs clang-format --verbose $cargs
    print_box "Running prettier"
    find integration -name '*.lua' | xargs prettier $pargs
}

if [ "$0" = "$BASH_SOURCE" ]; then
    format_flags "$@"
    format
fi

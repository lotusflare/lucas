#!/bin/sh -e

fix=false

print() {
    echo $1 | (command -v boxes > /dev/null && boxes -d stone || cat)
}

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
        pargs='-w'
        cargs='-i'
    else
        pargs='-c'
        cargs='-n'
    fi
    print "Running clang-format"
    find src include -name '*.h' -o -name '*.c' | xargs clang-format --verbose $cargs
    print "Running prettier"
    find integration -name '*.lua' | xargs prettier $pargs
}

if [ -n "$SKIP_BUILD" ]; then
    exit 0
fi

format_flags "$@"
format

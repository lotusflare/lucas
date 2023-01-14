#!/bin/sh -e

print() {
    echo $1 | (command -v boxes > /dev/null && boxes -d stone || cat)
}

if [ "$0" = "print.sh" ]; then
    print "$@"
fi

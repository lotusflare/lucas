#!/bin/bash -e

. ./print.sh

clean=false
build=false
test=false

clean() {
    if $clean; then
        print "Removing containers"
        docker compose rm -fs
    fi
}

build() {
    if $build; then
        print "Building containers"
        docker compose build --progress=tty
    fi
}

test() {
    if $test; then
        print "Running tests"
        docker compose run driver bash -c "busted; echo $?"
    fi
}

cassandra() {
    print "Waiting for Cassandra"
    docker compose up cassandra --wait --quiet-pull

    print "Seeding Cassandra"
    for file in $(find integration -name '*.cql' | sort); do
        cat $file | docker compose exec -T cassandra cqlsh
        echo $file
    done
}

run_flags() {
    while getopts 'cbt' option; do
        case $option in
        c) clean=true ;;
        b) build=true ;;
        t) test=true ;;
        ?) exit 1 ;;
        esac
    done
}

run() {
    clean
    build
    cassandra
    test
}

run_flags "$@"
run

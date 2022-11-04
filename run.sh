#!/bin/bash -e

clean=false
build=false
test=false

clean() {
    if $clean; then
        print_box "Removing containers"
        docker compose rm -fs
    fi
}

test() {
    if $test; then
        print_box "Running tests"
        docker compose run driver busted
    fi
}

build() {
    if $build; then
        print_box "Building containers"
        docker compose build --progress=tty
    fi
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
    print_box "Waiting for Cassandra"
    docker compose up cassandra --wait --quiet-pull
    print_box "Seeding Cassandra"
    find integration -name '*.cql' | sort | tee /dev/tty | xargs cat | docker compose exec -T cassandra cqlsh
    test
}

source format.sh
run_flags "$@"
run

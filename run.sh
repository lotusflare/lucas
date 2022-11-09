#!/bin/bash -e

clean=false
build=false
test=false

print() {
    echo $1 | (command -v boxes > /dev/null && boxes -d stone || cat)
}

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
        docker compose run driver busted
    fi
}

cassandra() {
    print "Waiting for Cassandra"
    docker compose up cassandra --wait --quiet-pull

    print "Seeding Cassandra"
    find integration -name '*.cql' | sort | tee /dev/stderr | xargs cat | docker compose exec -T cassandra cqlsh
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

#!/bin/sh -ex

clean=false

clean() {
    if $clean; then
        docker compose rm -fs
    fi
}

parse_flags() {
    while getopts 'c' option; do
        case $option in
            c) clean=true;;
            ?) exit 1;;
        esac
    done
}

run() {
    clean
    docker compose build
    docker compose up cassandra --wait
    find integration -name '*.cql' -s | xargs cat | docker compose exec -T cassandra cqlsh
    docker compose run driver busted
}

parse_flags "$@"
run

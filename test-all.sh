#!/bin/sh -e

clean=false

clean() {
    remove_containers
    # popd > /dev/null
}

remove_containers() {
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
    remove_containers
    docker compose build
    docker compose up cassandra --wait
    cat integration/init.cql | docker compose exec -T cassandra cqlsh
    docker compose run driver busted --output=TAP .
}

trap clean EXIT
# pushd integration > /dev/null
parse_flags "$@"
run

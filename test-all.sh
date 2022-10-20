#!/bin/sh

set -e
trap clean EXIT

pushd integration

docker compose rm -fs
docker compose up cassandra --wait

docker compose exec cassandra cqlsh -f /init.cql
docker compose exec driver busted --output=TAP .

function clean {
    echo "cleaning up"
    docker compose rm -fs
    popd
    if [[ ${exit_code} -ne 0 ]]; then
        echo "fail"
    fi
    if [[ ${exit_code} -eq 0 ]]; then
        echo "pass"
    fi
}

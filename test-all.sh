#!/bin/sh

function clean {
    popd > /dev/null
    if [[ ${exit_code} -ne 0 ]]; then
        echo "fail"
    fi
    if [[ ${exit_code} -eq 0 ]]; then
        echo "pass"
    fi
}

set -ex
trap clean EXIT
pushd integration > /dev/null

docker compose rm -fs
docker compose up cassandra --wait

docker compose exec cassandra cqlsh -f /init.cql
sleep 10
docker compose exec driver busted --output=TAP .

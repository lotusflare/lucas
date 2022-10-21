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

docker compos build
docker compose rm -fs
docker compose up cassandra --wait
cat init.cql | docker compose exec -T cassandra cqlsh
docker compose run driver busted --output=TAP .

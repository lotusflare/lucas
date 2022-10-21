#!/bin/sh -ex

function clean() {
    popd > /dev/null
}

trap clean EXIT
pushd integration > /dev/null

docker compose build
docker compose rm -fs
docker compose up cassandra --wait
cat init.cql | docker compose exec -T cassandra cqlsh
docker compose run driver busted --output=TAP .

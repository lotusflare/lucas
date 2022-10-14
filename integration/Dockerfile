FROM ubuntu:22.04

RUN apt-get update -y \
    && apt-get install -y luajit \
    && apt-get clean

COPY --from=ffi-driver /app/build/libcmake_cassandra_lua.so /usr/local/bin/

COPY tests /app/tests

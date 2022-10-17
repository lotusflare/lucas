FROM ubuntu:22.04

RUN apt-get update -y \
    && apt-get install -y luajit \
    && apt-get clean

COPY --from=ffi-driver /app/build/luacassandra.so /usr/local/bin/

ENV LUA_CPATH "/app/build/?.so;/usr/local/lib/lua/5.1/?.so"

COPY tests /app/tests

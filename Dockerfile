FROM ubuntu:22.04

RUN apt-get update -y \
    && apt-get install -y clang clangd-14 make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev luajit luarocks pkg-config \
    && apt-get clean \
    && ln -s /usr/bin/clangd-14 /usr/bin/clangd \
    && luarocks install busted

COPY . /app
WORKDIR /app/build
RUN cmake .. && make
WORKDIR /app

ENV LUA_CPATH "/app/build/?.so;/usr/local/lib/lua/5.1/?.so"

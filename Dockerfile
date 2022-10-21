FROM ubuntu:22.04

ENV LUA_CPATH "/app/build/?.so;/usr/local/lib/lua/5.1/?.so"

RUN apt-get update -y \
    && apt-get install -y clang clangd clang-format make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev luajit luarocks pkg-config npm \
    && apt-get clean \
    && luarocks install busted \
    && npm install --global prettier https://github.com/prettier/plugin-lua

COPY . /app
WORKDIR /app/build
RUN cmake .. \
    && make

WORKDIR /app
RUN ./format.sh

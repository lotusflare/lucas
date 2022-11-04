FROM ubuntu:20.04

ENV LUA_CPATH "/app/build/?.so;/usr/local/lib/lua/5.1/?.so"
ARG DEBIAN_FRONTEND="noninteractive"

RUN apt-get update -y \
    && apt-get install -y git clang clangd clang-format make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev luajit luarocks pkg-config nodejs npm \
    && apt-get clean \
    && luarocks install busted \
    && npm install --global prettier https://github.com/prettier/plugin-lua.git

COPY . /app
WORKDIR /app
RUN ./format.sh \
    && cmake -S . -B build \
    && cmake --build build

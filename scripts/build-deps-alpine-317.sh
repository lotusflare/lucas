#!/bin/sh

apk update
apk add --no-cache \
    autoconf \
    build-base \
    git \
    clang15 \
    clang15-extra-tools \
    cmake \
    openssl-dev \
    libuv-dev \
    zlib-dev \
    lua5.1 \
    lua5.1-dev \
    luajit-dev \
    luajit \
    luarocks \
    valgrind \
    gdb \
    doxygen \
    graphviz \
    bash

git config --global url.https://.insteadOf git://
/usr/bin/luarocks-5.1 install busted
/usr/bin/luarocks-5.1 install luasocket
/usr/bin/luarocks-5.1 install uuid
/usr/bin/luarocks-5.1 install lua-cassandra

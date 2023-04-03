#!/bin/bash

apt-get update
apt-get install -y \
  git \
  clang-10 \
  clang-format-10 \
  make \
  cmake \
  libssl-dev \
  libuv1-dev \
  zlib1g-dev \
  libluajit-5.1-dev \
  luajit \
  luarocks \
  pkg-config \
  valgrind \
  gdb \
  doxygen \
  graphviz
apt-get clean
ln -s /usr/bin/clang-format-10 /usr/bin/clang-format
ln -s /usr/bin/clang-10 /usr/bin/clang
git config --global url.https://.insteadOf git://
luarocks install busted
luarocks install luasocket
luarocks install uuid
luarocks install lua-cassandra

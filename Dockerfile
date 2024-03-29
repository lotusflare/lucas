# syntax=docker/dockerfile:1

FROM ubuntu:18.04 AS base

ENV LD_LIBRARY_PATH="/usr/local/lib/x86_64-linux-gnu"
ENV LUA_CPATH="/app/build/?.so;/usr/local/lib/lua/5.1/?.so;/usr/local/lib/x86_64-linux-gnu/?.so"
ENV LUA_PATH="/app/integration/tests/?.lua;;"
ARG DEBIAN_FRONTEND="noninteractive"
ARG CLANGD_TAG="15.0.6"
ARG STYLUA_TAG="v0.16.0"
ARG SHFMT_TAG="v3.6.0"
SHELL ["/bin/bash", "-euxo", "pipefail", "-c"]

RUN <<EOF
apt-get -qq -o=Dpkg::Use-Pty=0 update
apt-get -qq -o=Dpkg::Use-Pty=0 install \
  git \
  boxes \
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
EOF

COPY --from=mvdan/shfmt:v3-alpine /bin/shfmt /usr/bin/shfmt
# COPY --from=ghcr.io/JohnnyMorganz/StyLua:stylua:0.17.0 /stylua /usr/bin/stylua

COPY ./vendor /app/vendor/
WORKDIR /app/vendor/cpp-driver/build
RUN <<EOF
cmake ..
make
make install
EOF
WORKDIR /app

FROM base AS build
COPY . /app/
WORKDIR /app/build
RUN <<EOF
cmake ..
cmake --build .
EOF
WORKDIR /app
RUN ./format.sh

FROM scratch AS artifacts
COPY --from=build /app/build/lucas.so* /
COPY --from=build /app/vendor/cpp-driver/build/libcassandra.so* /

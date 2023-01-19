FROM ubuntu:20.04 AS base

ENV LD_LIBRARY_PATH="/usr/local/lib/x86_64-linux-gnu"
ENV LUA_CPATH="/app/build/?.so;/usr/local/lib/lua/5.1/?.so;/usr/local/lib/x86_64-linux-gnu/?.so"
ARG DEBIAN_FRONTEND="noninteractive"
SHELL ["/bin/bash", "-c"]

RUN apt-get -qq -o=Dpkg::Use-Pty=0 update \
    && apt-get -qq -o=Dpkg::Use-Pty=0 install git boxes clang clangd clang-format make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev luajit luarocks pkg-config \
    && apt-get clean \
    && wget -q https://github.com/JohnnyMorganz/StyLua/releases/download/v0.16.0/stylua-linux.zip \
    && unzip stylua-linux.zip -d /usr/bin \
    && rm stylua-linux.zip \
    && git config --global url.https://.insteadOf git:// \
    && luarocks install luasec \
    && luarocks install busted \
    && luarocks install luasocket \
    && luarocks install lua-cassandra

COPY ./vendor/ /app/vendor/
WORKDIR /app
RUN pushd vendor/cpp-driver/build \
    && cmake .. \
    && cmake --build . \
    && cmake --install . \
    && popd

FROM base AS build
COPY . /app/
RUN pushd build \
    && cmake .. \
    && cmake --build . \
    && popd \
    && ./format.sh

FROM scratch AS artifacts
COPY --from=build /app/build/lucas.so* /
COPY --from=build /usr/local/lib/x86_64-linux-gnu/libcassandra.so* /

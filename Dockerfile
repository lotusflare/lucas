FROM ubuntu:20.04 AS base

ENV LD_LIBRARY_PATH="/usr/local/lib/x86_64-linux-gnu"
ENV LUA_CPATH="/app/build/?.so;/usr/local/lib/lua/5.1/?.so;/usr/local/lib/x86_64-linux-gnu/?.so"
ARG DEBIAN_FRONTEND="noninteractive"

RUN apt-get -qq -o=Dpkg::Use-Pty=0 update \
    && apt-get -qq -o=Dpkg::Use-Pty=0 install git boxes clang clangd clang-format make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev luajit luarocks pkg-config \
    && apt-get clean \
    && wget -q https://github.com/JohnnyMorganz/StyLua/releases/download/v0.15.3/stylua-linux.zip \
    && unzip stylua-linux.zip -d /usr/bin \
    && rm stylua-linux.zip \
    && git config --global url.https://.insteadOf git:// \
    && luarocks install luasec \
    && luarocks install busted \
    && luarocks install luasocket \
    && luarocks install lua-cassandra

COPY ./vendor/ /app/vendor/
WORKDIR /app/vendor/cpp-driver/build
RUN cmake .. \
    && cmake --build . \
    && cmake --install .

FROM base AS build
COPY . /app/
WORKDIR /app
RUN ./format.sh
WORKDIR /app/build
RUN cmake .. \
    && cmake --build .

FROM scratch AS artifacts
COPY --from=build /app/build/lucas.so* /
COPY --from=build /usr/local/lib/x86_64-linux-gnu/libcassandra.so* /

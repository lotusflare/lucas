FROM ubuntu:20.04

ENV LUA_CPATH="/app/build/?.so;/usr/local/lib/lua/5.1/?.so"
ARG DEBIAN_FRONTEND="noninteractive"
ARG SKIP_BUILD=""

RUN apt-get -qq -o=Dpkg::Use-Pty=0 update \
    && apt-get -qq -o=Dpkg::Use-Pty=0 install git boxes clang-12 clangd-12 clang-format-12 make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev luajit luarocks pkg-config nodejs npm \
    && apt-get clean \
    && git config --global url.https://.insteadOf git:// \
    && luarocks install luasec \
    && luarocks install busted \
    && luarocks install luasocket \
    && luarocks install lua-cassandra \
    && npm install --global prettier https://github.com/prettier/plugin-lua.git \
    && ln -s /usr/bin/clang-format-12 /usr/bin/clang-format

COPY ./vendor/ /app/vendor/
WORKDIR /app/vendor/cpp-driver
RUN mkdir -p build \
    && cd build \
    && cmake .. \
    && make \
    && make install

COPY . /app/
WORKDIR /app
RUN ./format.sh \
    && ./build.sh

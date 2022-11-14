FROM ubuntu:20.04

ENV LUA_CPATH="/app/build/?.so;/usr/local/lib/lua/5.1/?.so"
ARG DEBIAN_FRONTEND="noninteractive"
ARG SKIP_BUILD="false"

RUN apt-get update -yq \
    && apt-get install -yq git boxes clang-12 clangd-12 clang-format-12 make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev luajit luarocks pkg-config nodejs npm \
    && apt-get clean \
    && luarocks install busted \
    && npm install --global prettier https://github.com/prettier/plugin-lua.git \
    && ln -s /usr/bin/clang-format-12 /usr/bin/clang-format

COPY . /app
WORKDIR /app
RUN if [ ${SKIP_BUILD} = false ]; then \
        ./format.sh \
        && cmake -S . -B build \
        && cmake --build build; \
    fi;

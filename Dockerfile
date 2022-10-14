FROM ubuntu:22.04

RUN apt-get update -y \
    && apt-get install -y g++ make cmake libuv1-dev libssl-dev zlib1g-dev zlib1g-dev pkg-config \
    && apt-get clean

COPY . /app

# Build LuaJIT manually because it is not a CMake project
WORKDIR /app/vendor/LuaJIT
RUN make \
    && make install

WORKDIR /app/build
RUN cmake .. \
    && make

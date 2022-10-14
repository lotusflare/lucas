FROM ubuntu:22.04

RUN apt-get update -y \
    && apt-get install -y g++ make cmake libssl-dev libuv1-dev zlib1g-dev pkg-config \
    && apt-get clean

COPY . /app

WORKDIR /app/vendor/LuaJIT
RUN make \
    && make install

WORKDIR /app/build
RUN cmake .. \
  && make

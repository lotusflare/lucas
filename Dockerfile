FROM ubuntu:22.04

RUN apt-get update -y \
    && apt-get install -y g++ make cmake libssl-dev libuv1-dev zlib1g-dev libluajit-5.1-dev pkg-config \
    && apt-get clean

COPY . /app
WORKDIR /app/build
RUN cmake .. \
  && make

#!/bin/bash

apt-get update
apt-get install -y python3-pip
apt-get clean
CASS_DRIVER_BUILD_CONCURRENCY=`nproc` pip3 install cassandra-driver
pip3 install cqlsh
cqlsh -v

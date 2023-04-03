#!/bin/bash
set -e

mkdir -p /app/build
cd /app/build
cmake ..
cmake --build .

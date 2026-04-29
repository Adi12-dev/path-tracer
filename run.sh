#!/bin/bash

set -e

mkdir -p build
cd build

cmake ..
make -j$(nproc --ignore=1)

cd ..

./build/app
#!/bin/bash

set +eux

script_dir=$(dirname $0)

i3_ipcpp_dir=${script_dir}/../i3-ipcpp

mkdir -p build
g++ -c -std=c++17 -I${i3_ipcpp_dir}/include main.cpp -o build/main.o
g++ -static -L${i3_ipcpp_dir}/build/lib/static build/main.o -o build/i3-resize -li3-ipc++

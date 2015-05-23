#!/bin/bash
if [[ $# > 0 ]]; then
dir="$1"
mkdir -p $1/build
cd $1/build
cmake ..
make rend
make main
fi

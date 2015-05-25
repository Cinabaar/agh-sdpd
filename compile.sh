#!/bin/bash
if [[ $# > 0 ]]; then
mkdir -p $1/build
cd $1/build
cmake ..
make rend
make main
fi

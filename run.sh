#!/bin/bash
if [[ $# > 1 ]]; then
    cd $1/bin
    mpirun -np $2 main
fi

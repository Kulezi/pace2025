#!/bin/bash

if [[ $# -lt 3 ]]; then
    echo "usage: ./runner.sh <suffix_for_solutions> <instance_directory> <time_limit_in_seconds> [flags to pass to main.out]"
    exit 1
fi

SUFFIX=$1
INSTANCES=$2
TIME_LIMIT_SECONDS=$3
shift 3
FLAGS="$@"


N_CORES=$(nproc --all)

cmake .
make main

ls $INSTANCES/*.gr | parallel --halt never -j${N_CORES} "./run.sh {} ${TIME_LIMIT_SECONDS} ${SUFFIX} $FLAGS" || true


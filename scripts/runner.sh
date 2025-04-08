#!/bin/bash

if [[ $# -lt 4 ]]; then
    echo "usage: ./runner.sh <exectuable> <suffix_for_solutions> <instance_directory> <time_limit_in_seconds> [flags to pass to executable]"
    echo "runs ./main.out with given flags on all instances from given directory,"
    echo "storing the results in .solutions/<path_to_instance>.{report, sol, err}"
    exit 1
fi

EXECUTABLE=$1
SUFFIX=$2
INSTANCES=$3
TIME_LIMIT_SECONDS=$4
shift 4
FLAGS="$@"


N_CORES=$(nproc --all)


ls $INSTANCES/*.gr | parallel --halt never -j${N_CORES} "./run.sh ${EXECUTABLE} {} ${TIME_LIMIT_SECONDS} ${SUFFIX} $FLAGS" || true


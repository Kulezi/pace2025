#!/bin/bash

INSTANCES="in/PACE2025-instances/ds/exact"
SOLUTIONS="${INSTANCES}/.solutions"
N_CORES=$(nproc --all)
TIME_LIMIT_SECONDS=1

cmake .
make eval

rm -rf $SOLUTIONS
mkdir $SOLUTIONS

basename -a in/PACE2025-instances/ds/exact/*.gr | parallel --halt never -j${N_CORES} "./eval.sh {} $TIME_LIMIT_SECONDS" || true

printf "file,time(s),memory(MB)"
echo "p ds 0 0" | ./eval.out $SOLUTIONS/empty.gr >/dev/null
ls $SOLUTIONS/*.content | xargs cat


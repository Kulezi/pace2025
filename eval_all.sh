#!/bin/bash

INSTANCES="in/PACE2025-instances/ds/exact"
SOLUTIONS="${INSTANCES}/.solutions"

cmake .
make eval

rm -rf $SOLUTIONS
mkdir $SOLUTIONS

basename -a in/PACE2025-instances/ds/exact/*.gr | parallel --halt never -j2 "./eval.sh {}" || true


echoerr() { echo "$@" 1>&2; }

echoerr -ne "file" 
echo "p ds 0 0" | ./eval.out $SOLUTIONS/empty.gr >/dev/null
ls $SOLUTIONS/*.content | xargs cat


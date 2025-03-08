#!/bin/bash

INSTANCES="in/PACE2025-instances/ds/exact"
SOLUTIONS="${INSTANCES}/.solutions"

cmake .
make eval

rm -rf $SOLUTIONS
mkdir $SOLUTIONS

basename -a in/PACE2025-instances/ds/exact/*.gr | parallel -j2 "./eval.sh {}"

ls $SOLUTIONS/*.header | head -n 1 | xargs cat
ls $SOLUTIONS/*.content | xargs cat


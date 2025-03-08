#!/bin/bash

INSTANCES="in/PACE2025-instances/ds/exact"
SOLUTIONS="${INSTANCES}/.solutions"

cmake .
make eval

rm -rf $SOLUTIONS
mkdir $SOLUTIONS

basename -a in/PACE2025-instances/ds/exact/*.gr | parallel -j2 "./eval.sh {}"

echo -ne "file" 
echo "p ds 0 0" | ./eval.out >/dev/null
ls $SOLUTIONS/*.content | xargs cat


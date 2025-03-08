#!/bin/bash
if [[ $# -ne 1 ]]; then
    echo "usage: ./eval.sh <graph.gr>"
    exit 1
fi

INSTANCES="in/PACE2025-instances/ds/exact"
SOLUTIONS="${INSTANCES}/.solutions"
TIMEOUT_SECONDS=60

echo "Running test $1"

echo -ne $1 > ${SOLUTIONS}/$1.content
echo -ne file > ${SOLUTIONS}/$1.header
timeout $TIMEOUT_SECONDS ./eval.out < ${INSTANCES}/$1 >> ${SOLUTIONS}/$1.content 2> ${SOLUTIONS}/$1.header
printf "\n" >>  ${SOLUTIONS}/$1.content
printf "\n" >>  ${SOLUTIONS}/$1.header
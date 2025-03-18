#!/bin/bash
if [[ $# -ne 2 ]]; then
    echo "usage: ./eval.sh <graph.gr> <timeout_in_seconds>"
    exit 1
fi

INSTANCES="in/PACE2025-instances/ds/exact"
SOLUTIONS="${INSTANCES}/.solutions"
TIMEOUT_SECONDS=$2

echo "Running test $1"

CONTENT=${SOLUTIONS}/$1.content
HEADER=${SOLUTIONS}/$1.header

echo -ne $1 > $CONTENT
echo -ne file > $HEADER
timeout $TIMEOUT_SECONDS ./eval.out ${SOLUTIONS}/$1 < ${INSTANCES}/$1 >> $CONTENT 2>> $HEADER
printf "\n" >>  $CONTENT
printf "\n" >>  $HEADER
#!/bin/bash
if [[ $# -ne 2 ]]; then
    echo "usage: ./eval.sh <graph.gr> <timeout_in_seconds>"
    exit 1
fi

INSTANCES="in/PACE2025-instances/ds/exact"
SOLUTIONS="${INSTANCES}/.solutions"
TIMEOUT_SECONDS=$2
TIME_OUTPUT=$SOLUTIONS/$1.time
TIMEOUT="timeout -s INT $TIMEOUT_SECONDS"
TIME="/usr/bin/time --quiet -f seconds=%e\nkbytes=%M -o $TIME_OUTPUT"

echo "Running test $1"

CONTENT=${SOLUTIONS}/$1.content
TEMP_CONTENT=$(mktemp)
HEADER=${SOLUTIONS}/$1.header

printf "file,time(s),memory(MB)" > $HEADER
printf $1 > $CONTENT
$TIMEOUT $TIME ./eval.out ${SOLUTIONS}/$1 < ${INSTANCES}/$1 >> $TEMP_CONTENT 2>> $HEADER

source $TIME_OUTPUT
printf "\n" >> $HEADER
printf ",${seconds},$(expr ${kbytes} / 1024)" >> $CONTENT

cat $TEMP_CONTENT >> $CONTENT
printf "\n" >> $CONTENT

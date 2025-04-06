#!/bin/bash
set -e

if [[ $# -lt 3 ]]; then
    echo "usage: ./eval.sh <graph.gr> <timeout_in_seconds> <suffix> [flags to pass to main.out]"
    exit 1
fi

SOLUTIONS=".solutions"
GRAPH_FILE=$1
TIMEOUT_SECONDS=$2
SUFFIX=$3

shift 3
FLAGS="$@"

mkdir -p ${SOLUTIONS}/${GRAPH_FILE}

OUTPUT_DIR=${SOLUTIONS}/${GRAPH_FILE}

TIME_OUTPUT=${OUTPUT_DIR}/${SUFFIX}.time
TIMEOUT="timeout -s INT $TIMEOUT_SECONDS"


SOL=${OUTPUT_DIR}/${SUFFIX}.sol
ERR=${OUTPUT_DIR}/${SUFFIX}.err
REPORT=${OUTPUT_DIR}/${SUFFIX}.report
TIME=(/usr/bin/time --quiet -f "seconds %e\nkbytes %M" -o ${REPORT})

echo "Running test $GRAPH_FILE"
set -x
$TIMEOUT "${TIME[@]}" ./main.out $FLAGS --input_file ${GRAPH_FILE} > ${SOL} 2> ${ERR} || true

# teraz dopisujemy
if [[ -s ${SOL} ]]; then
    first_line=$(head -n 1 ${SOL})
    if [[ $(wc -w ${first_line}) -ne 1 ]]; then
        echo "ds_size ${first_line}" >> ${REPORT}
    else 
        echo "ds_size UNAPPLICABLE" >> ${REPORT}
    fi
else
    echo "ds_size FAILED" >> ${REPORT}
fi
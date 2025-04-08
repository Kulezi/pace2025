#!/bin/bash
set -e

if [[ $# -lt 4 ]]; then
    echo "usage: ./run.sh <executable> <graph.gr> <timeout_in_seconds> <suffix> [flags to pass to executable]"
    echo "runs ./main.out with given flags on given instance,"
    echo "storing the results in .solutions/<path_to_instance>.{report, sol, err}"
    exit 1
fi

SOLUTIONS=".solutions"
EXECUTABLE=$1
GRAPH_FILE=$2
GRAPH_NAME=$(basename ${GRAPH_FILE})
TIMEOUT_SECONDS=$3
SUFFIX=$4

shift 4
FLAGS="$@"

OUTPUT_DIR=${SOLUTIONS}/${GRAPH_NAME}
mkdir -p ${OUTPUT_DIR}


TIME_OUTPUT=${OUTPUT_DIR}/${SUFFIX}.time
TIMEOUT="timeout -s INT $TIMEOUT_SECONDS"


SOL=${OUTPUT_DIR}/${SUFFIX}.sol
ERR=${OUTPUT_DIR}/${SUFFIX}.err
REPORT=${OUTPUT_DIR}/${SUFFIX}.report
TIME=(/usr/bin/time --quiet -f "seconds %e\nkbytes %M" -o ${REPORT})

echo $OUTPUT_DIR

echo "${SUFFIX}: Running test ${GRAPH_FILE}"
$TIMEOUT "${TIME[@]}" ./${EXECUTABLE} $FLAGS --input_file ${GRAPH_FILE} > ${SOL} 2> ${ERR} || true

if [[ -s ${SOL} ]]; then
    first_line=$(head -n 1 ${SOL})
    if [[ $(echo ${first_line} | wc -w) -ne 1 ]]; then
        echo "ds_size UNAPPLICABLE" >> ${REPORT}
    else 
        echo "ds_size ${first_line}" >> ${REPORT}
    fi
else
    echo "ds_size FAILED" >> ${REPORT}
fi
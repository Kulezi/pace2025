#!/bin/bash

if [[ $# -lt 3 ]]; then
    echo "usage: ./run_all.sh <executable> <directory> <seconds>"
    exit 1
fi

EXECUTABLE=$1
INSTANCES=$(cd $2; pwd)
TIME_LIMIT_SECONDS=$3

./runner.sh $1 presolve ${INSTANCES} ${TIME_LIMIT_SECONDS} --mode presolve
./runner.sh $1 defaults ${INSTANCES} ${TIME_LIMIT_SECONDS}

# ./runner.sh $1 treewidth_dp ${INSTANCES} ${TIME_LIMIT_SECONDS} --solver treewidth_dp
# ./runner.sh $1 gurobi ${INSTANCES} ${TIME_LIMIT_SECONDS} --solver gurobi
# ./runner.sh $1 vc ${INSTANCES} ${TIME_LIMIT_SECONDS} --solver vc
# ./runner.sh $1 tw_exp ${INSTANCES} ${TIME_LIMIT_SECONDS} --mode treewidth


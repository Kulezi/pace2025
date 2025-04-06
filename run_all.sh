#!/bin/bash
INSTANCES="in/tiny/testset"

./runner.sh presolve ${INSTANCES} 1 --mode presolve
./runner.sh treewidth_dp ${INSTANCES} 1 --solver treewidth_dp
./runner.sh gurobi ${INSTANCES} 1 --solver gurobi
./runner.sh vc ${INSTANCES} 1 --solver vc


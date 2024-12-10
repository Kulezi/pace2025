#!/bin/bash

tests=(20 50 100 150 200 250 300)
mkdir .solutions
for n in ${tests[@]}; do
    echo -ne "\nbremen_subgraph_$n: "
    timeout 300 /usr/bin/time -o tim.log ./bench.out < in/tiny/testset/bremen_subgraph_$n.gr 2>.solutions/bench$n.log
    cat tim.log
done
#!/bin/bash

BENCHES="in/bench"

mkdir .solutions
for i in $BENCHES/*.gr; do
    name=$(basename $i)
    echo $i
    timeout -v 60 /usr/bin/time -o tim.log ./bench.out < $i 2>.solutions/bench$name.log
    cat tim.log
    cat .solutions/bench$name.log | head -n 1
    echo ""
done

tests=(20 50 100 150 200 250 300)
for n in ${tests[@]}; do
    echo -ne "\nbremen_subgraph_$n: "
    timeout 300 /usr/bin/time -o tim.log ./bench.out < in/tiny/testset/bremen_subgraph_$n.gr 2>.solutions/bench$n.log
    cat tim.log
    cat .solutions/bench$n.log
done
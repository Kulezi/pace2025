#!/bin/bash
cmake .
make recon
rm $1
for test in in/PACE2025-instances/*; do
    echo -ne "$test" >> $1
    timeout 30 ./recon.out < $test >> $1
    printf "\n" >> $1
    # echo -ne "treewidth: " >> $1
    # sed -e 's/ds/tw/g' $test | timeout 10 ../htd/bin/htd_main --output width >> $1
done
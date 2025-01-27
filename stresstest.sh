# Stresstests the main solution on random instances
# against a brute force solution that considers all possible sets.
cmake .
make

for i in $(seq 1 10000); do
    echo -ne "$i "
    # Generate random input
    ./random_graph_gen.out $i >rg.gr
    if [ $? -ne 0 ]; then
        echo "Runtime error in rg.out"
        exit 1
    fi

    # Run brute force solution
    ./brute.out < rg.gr  >brute.sol
    if [ $? -ne 0 ]; then
        echo "Runtime error in brute.out"
        cat rg.gr
        exit 1
    fi

    ./out_verifier.out rg.gr brute.sol > verifier.log
    if [ $? -ne 0 ]; then
        echo "Incorrect ds in brute.out"
        cat rg.gr
        cat main.log
        cat verifier.log
        exit 1
    fi

    # Run main solution
    ./main.out <rg.gr 2>main.log >main.sol 
    if [ $? -ne 0 ]; then
        echo "Runtime error in main.out"
        cat rg.gr
        cat main.log
        exit 1
    fi

    ./out_verifier.out rg.gr main.sol > verifier.log
    if [ $? -ne 0 ]; then
        echo "Incorrect ds in main.out"
        cat rg.gr
        cat main.log
        cat verifier.log
        exit 1
    fi

    head -n 1 main.sol > main.count
    head -n 1 brute.sol > brute.count
    # Check whether bruteforce found a dominating set of the same size.
    diff brute.count main.count
    if [ $? -ne 0 ]; then
        echo "Mismatch found!"
        cat rg.gr
        cat main.log
        exit 1
    fi
done

echo "[OK] Stress test done"
make clean

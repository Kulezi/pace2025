for i in $(seq 1 10000); do
    # Generate random input
    ./random_graph_gen.out $i >rg.gr
    if [ $? -ne 0 ]; then
        echo "Runtime error in rg.out"
        exit 1
    fi

    # Run brute force solution
    ./brute.out <rg.gr | head -n 1 >brute.ans
    if [ $? -ne 0 ]; then
        echo "Runtime error in brute.out"
        cat rg.gr
        exit 1
    fi

    # Run main solution
    ./main.out <rg.gr 2>main.log | head -n 1 >main.ans 
    if [ $? -ne 0 ]; then
        echo "Runtime error in main.out"
        cat rg.gr
        cat main.log
        exit 1
    fi

    # Compare outputs
    diff brute.ans main.ans
    if [ $? -ne 0 ]; then
        echo "Mismatch found!"
        cat rg.gr
        cat main.log
        exit 1
    fi
done

echo "[OK] Stress test done"
make clean

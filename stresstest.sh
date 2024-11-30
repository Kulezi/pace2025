for i in $(seq 1 1000); do
    printf "\nRunning test $i\n"
    ./rg.out >rg.gr
    ./brute.out <rg.gr | head -n 1 >brute.ans
    ./main.out <rg.gr | head -n 1 >main.ans
    diff brute.ans main.ans
    if [ $? -ne 0 ]; then
        cat rg.gr
        exit 1
    fi
done

echo "[OK] Stresstest done"
make clean
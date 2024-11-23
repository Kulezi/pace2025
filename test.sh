for i in $(seq 1 1000); do
    echo $i
    ./rg.out >rg.gr
    ./brute.out <rg.gr | head -n 1 >brute.ans
    ./main.out <rg.gr | head -n 1 >main.ans
    diff brute.ans main.ans || exit 1
done

for test in in/tw/*; do
    echo "Running $test:"
    timeout 30 ./main.out <$test >ds.ans
    ./out_verifier.out $test ds.ans
done

make clean
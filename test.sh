# for i in $(seq 1 1000); do
#     echo $i
#     ./rg.out >rg.gr
#     ./brute.out <rg.gr | head -n 1 >brute.ans
#     ./main.out <rg.gr | head -n 1 >main.ans
#     diff brute.ans main.ans || exit 1
# done

sol_dir=".solutions"

for test in in/tiny/testset/*; do
    echo "Running $test:"
    name=$(basename $test)
    timeout 600 ./main.out <$test >$sol_dir/$name.sol 2>$sol_dir/$name.log
    ./out_verifier.out $test $sol_dir/$name.sol
done

make clean
rm rg.gr
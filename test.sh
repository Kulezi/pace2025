if [ $# -ne 1 ]; then
    echo "usage: ./test.sh <test_dir>"
    exit 1
fi

sol_dir=".solutions"
test_dir=$1

for test in $test_dir/*; do
    echo "Running $test:"
    name=$(basename $test)
    timeout 600 ./main.out <$test >$sol_dir/$name.sol 2>$sol_dir/$name.log
    ./out_verifier.out $test $sol_dir/$name.sol
done

make clean
rm rg.gr

for test in in/tw/*; do
    timeout 2 ./main.out < $test > ds.out
    ./out_verifier.out $test ds.out
    echo "$test - |ds| = $(head -n 1 ds.out)"
done

for test in in/tw/*; do
    timeout 2 ./main < $test > ds.out
    ./out_verifier $test ds.out
    echo "$test - |ds| = $(head -n 1 ds.out)"
done
TESTS = src/in
# CFLAGS = -O3 -g -Wall -Wextra -pedantic -Wshadow -Wformat=2 -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector
CFLAGS = -O3 -g -Wall
LDFLAGS = -L/usr/local/lib -lhtd
build:
	g++ $(CFLAGS)  src/main.cpp -o main.out $(LDFLAGS)
	g++ $(CFLAGS) src/brute.cpp -o brute.out $(LDFLAGS)
	g++ $(CFLAGS) src/random_graph_gen.cpp -o rg.out

.PHONY: test
test: build
	g++ $(CFLAGS) $(LDFLAGS) src/out_verifier.cpp -o out_verifier.out
	@ ./test.sh in/tiny/testset

.PHONY: stresstest
stresstest: build
	@ ./stresstest.sh

.PHONY: bench
bench: 
	g++ $(CFLAGS) -DBENCH src/bench.cpp -o bench.out $(LDFLAGS) 
	@ ./bench.sh

clean:
	rm *.out
	rm *.ans

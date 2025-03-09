# PACE Challenge 2025 - Dominating Set
This is an exact solver for the [Dominating Set](https://pacechallenge.org/2025/ds/) problem for [PACE Challenge 2025](https://pacechallenge.org/2025/)

Dependencies:
- g++ version supporting C++23
- CMake 3.10

### Building
`cmake . && make main`

### Running
`./main.out < <instance.gr>`

The resulting dominating set will be printed to stdout.

### Running benchmarks
`./bench.sh`

### Running stresstests
`./stresstest.py`

### Generating an evaluation matrix from all instance runs
`./eval_all.sh`
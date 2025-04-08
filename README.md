# PACE Challenge 2025 - Dominating Set
This is an exact solver for the [Dominating Set](https://pacechallenge.org/2025/ds/) problem for [PACE Challenge 2025](https://pacechallenge.org/2025/)

### Dependencies
Dependencies:
- g++ version supporting C++23
- CMake 3.10
- Gurobi
- FlowCutter (PACE 2017)
- WeGotYouCovered (PACE 2019)


### Building
```
./scripts/install_dependencies.sh
cmake -B build && cd build && make
```

### Running
`./main.out < <instance.gr>`

The resulting dominating set will be printed to stdout.

For more options run `./main.out --help`

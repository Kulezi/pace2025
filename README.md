# PACE Challenge 2025 - Dominating Set
This is a student submission for the exact track of the [Dominating Set](https://pacechallenge.org/2025/ds/) problem.
for [PACE Challenge 2025](https://pacechallenge.org/2025/).

### Dependencies
Dependencies:
- g++ version supporting C++20
- CMake 3.10
- [FlowCutter (PACE 2017)](https://github.com/Kulezi/flow-cutter-pace17/tree/6cae71e55858faccc59931bae0debda56a6c226b)
- [Peaty (PACE 2019)](https://github.com/Kulezi/peaty/)

### Building
Running following commands will produce an executable `main.out` in the `release/` directory.

```
git clone github.com/Kulezi/pace2025
cd pace2025 && git submodule update --init --recursive
./scripts/install_dependencies.sh
cd release && cmake -DCMAKE_BUILD_TYPE=Release .. && make
```

The submission also contains a Dockerfile meant to be substituted for this file
in the [Docker](https://github.com/MarioGrobler/PACE2025-docker/blob/master/pace-eval-ds-exact/Dockerfile) setup provided by PACE2025 Commitee
### Running
`./main.out < <instance.gr>`

The resulting dominating set will be printed to stdout.

For more options run `./main.out --help`





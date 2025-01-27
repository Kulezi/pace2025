# PACE Challenge 2025 - Dominating Set
This is an exact solver for the [Dominating Set](https://pacechallenge.org/2025/ds/) problem for [PACE Challenge 2025](https://pacechallenge.org/2025/)

Dependencies:
- g++
- CMake
- [htd](https://github.com/mabseher/htd) <details>
    <summary> How to install </summary>
    CMakeLists.txt assumes its installation is located in /usr/local/lib and /usr/local/include

        
        git clone https://github.com/mabseher/htd/
        cd htd
        cmake .
        sudo make
        sudo make install
        
    

    </summary>
    </details>


### Building
`cmake . && make main`

### Running
`./main.out < <instance.gr>`

The resulting dominating set will be printed to stdout.

### Running benchmarks

`./bench.sh`

### Running stresstests
`./stresstest.sh`
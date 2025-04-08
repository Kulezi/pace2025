cmake_minimum_required(VERSION 3.10)

project(SourceCode)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "-Wall -g -O3 -fPIC")

# Find required packages
find_package(Threads REQUIRED)

# Add the pace-2019 library as a subdirectory
set(PACE_2019_DIR src/ext/pace-2019)
add_subdirectory(${PACE_2019_DIR} EXCLUDE_FROM_ALL)

# Include the src directory
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/ext/flow-cutter-pace17)
include_directories(${CMAKE_SOURCE_DIR}/src/ext/gurobi1201/linux64/include)
include_directories(${CMAKE_SOURCE_DIR}/src/ext/pace-2019/lib)

# Set Gurobi library paths
set(GUROBI_DIR "${CMAKE_SOURCE_DIR}/src/ext/gurobi1201/linux64")
set(GUROBI_CXX_LIB "${GUROBI_DIR}/lib/libgurobi_c++.a")
set(GUROBI_SHARED_LIB "${GUROBI_DIR}/lib/libgurobi120.so")

# Create imported targets for Gurobi
add_library(gurobi_c++ STATIC IMPORTED)
set_target_properties(gurobi_c++ PROPERTIES
    IMPORTED_LOCATION ${GUROBI_CXX_LIB}
)

add_library(gurobi_shared SHARED IMPORTED)
set_target_properties(gurobi_shared PROPERTIES
    IMPORTED_LOCATION ${GUROBI_SHARED_LIB}
)

# Set the executable suffix to .out
set(CMAKE_EXECUTABLE_SUFFIX ".out")

# Define the external library separately (suppress warnings here explicitly)
add_library(flow_cutter STATIC
    src/ext/flow-cutter-pace17/src/cell.cpp
    src/ext/flow-cutter-pace17/src/greedy_order.cpp
    src/ext/flow-cutter-pace17/src/list_graph.cpp
    src/ext/flow-cutter-pace17/src/pace.cpp
    src/ext/flow-cutter-pace17/src/tree_decomposition.cpp
)

# List of implementation files for header files
set(HEADER_IMPLEMENTATIONS
    src/dshunter/solver/solver.cpp
    src/dshunter/solver/treewidth/td/flow_cutter_wrapper.cpp
    src/dshunter/instance.cpp
    src/dshunter/solver/treewidth/td/nice_tree_decomposition.cpp
    src/dshunter/solver/treewidth/td/rooted_tree_decomposition.cpp
    src/dshunter/rrules.cpp
    src/dshunter/solver/treewidth/ternary.cpp
    src/dshunter/solver/treewidth/td/tree_decomposition.cpp
    src/dshunter/solver/treewidth/treewidth_solver.cpp
    src/dshunter/solver/branching/branching_solver.cpp
    src/dshunter/solver/bruteforce/bruteforce_solver.cpp
    src/dshunter/solver/vc/vc_solver.cpp
    src/dshunter/solver/gurobi/gurobi_solver.cpp
    src/dshunter/solver/verifier.cpp
    src/dshunter/utils.cpp
)


# Create a library for shared components
add_library(core STATIC ${HEADER_IMPLEMENTATIONS})

# Include directories for core library
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Link against necessary libraries (order is important: C++ wrapper first, then shared lib)
target_link_libraries(core PUBLIC
    flow_cutter
    pace2019-vc
    gurobi_c++
    gurobi_shared
)

# List of source files for each executable
set(SOURCE_FILES
    src/main.cpp
    src/dshunter/testing/random_graph_gen.cpp
    src/eval.cpp
    src/dshunter/testing/small_graph_test.cpp
)

# Define each executable separately
foreach(SRC ${SOURCE_FILES})
    # Get the executable name (based on the source file name)
    get_filename_component(EXEC_NAME ${SRC} NAME_WE)

    # Create the executable for each source file
    add_executable(${EXEC_NAME} ${SRC})

    target_link_libraries(${EXEC_NAME} PRIVATE core)

    target_compile_definitions(${EXEC_NAME} PRIVATE DEBUG_MODE)

    # For the 'bench' and 'eval' targets, define DS_BENCHMARK
    if(EXEC_NAME STREQUAL "bench" OR EXEC_NAME STREQUAL "eval")
        target_compile_definitions(${EXEC_NAME} PRIVATE DS_BENCHMARK)
    endif()
endforeach()

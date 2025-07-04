cmake_minimum_required(VERSION 3.10)
project(DSHunter)

# Set C++ standard and common flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -g -O3 -fPIC -Wno-sign-compare")

# Find required packages
find_package(Threads REQUIRED)

# Add the pace-2019 library as a subdirectory
set(PEATY_DIR src/ext/peaty)
add_subdirectory(${PEATY_DIR} EXCLUDE_FROM_ALL)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/ext/flow-cutter-pace17)
include_directories(${CMAKE_SOURCE_DIR}/src/ext/peaty)

# Set the executable suffix to .out
set(CMAKE_EXECUTABLE_SUFFIX ".out")

# Define the external library (suppress warnings explicitly)
add_library(flow_cutter STATIC
        src/ext/flow-cutter-pace17/src/cell.cpp
        src/ext/flow-cutter-pace17/src/greedy_order.cpp
        src/ext/flow-cutter-pace17/src/list_graph.cpp
        src/ext/flow-cutter-pace17/src/pace.cpp
        src/ext/flow-cutter-pace17/src/tree_decomposition.cpp
)

# Define the external library (suppress warnings explicitly)
add_library(peaty STATIC
        src/ext/peaty/colourer.cpp
        src/ext/peaty/graph_colour_solver.cpp
        src/ext/peaty/graph.cpp
        src/ext/peaty/root_node_processing.cpp
        src/ext/peaty/sequential_solver.cpp
        src/ext/peaty/solve_mwc.cpp
        src/ext/peaty/sparse_graph.cpp
        src/ext/peaty/util.cpp
)

# List of implementation files for header files
set(HEADER_IMPLEMENTATIONS
        src/dshunter/bounds.cpp
        src/dshunter/instance.cpp
        src/dshunter/utils.cpp

        src/dshunter/solver/solver.cpp
        src/dshunter/solver/verifier.cpp

        src/dshunter/rrules/alber/main_rule_1.cpp
        src/dshunter/rrules/alber/main_rule_2.cpp
        src/dshunter/rrules/alber/simple_rule_1.cpp
        src/dshunter/rrules/alber/simple_rule_2.cpp
        src/dshunter/rrules/alber/simple_rule_3.cpp
        src/dshunter/rrules/alber/simple_rule_4.cpp
        src/dshunter/rrules/force/force_edge_rule.cpp
        src/dshunter/rrules/disregard/disregard_rule.cpp
        src/dshunter/rrules/disregard/remove_disregarded_rule.cpp
        src/dshunter/rrules/disregard/single_dominator_rule.cpp
        src/dshunter/rrules/dominate/same_dominators_rule.cpp
        src/dshunter/rrules/local/local_bruteforce.cpp
        src/dshunter/rrules/reduce.cpp
        src/dshunter/rrules/apply.cpp
        src/dshunter/rrules/defaults.cpp

        src/dshunter/solver/treewidth/ternary.cpp
        src/dshunter/solver/treewidth/treewidth_solver.cpp
        src/dshunter/solver/treewidth/td/flow_cutter_decomposer.cpp
        src/dshunter/solver/treewidth/td/exec_decomposer.cpp
        src/dshunter/solver/treewidth/td/decomposer.cpp
        src/dshunter/solver/treewidth/td/tree_decomposition.cpp
        src/dshunter/solver/treewidth/td/rooted_tree_decomposition.cpp
        src/dshunter/solver/treewidth/td/nice_tree_decomposition.cpp


        src/dshunter/solver/branching/branching_solver.cpp

        src/dshunter/solver/bruteforce/bruteforce_solver.cpp

        src/dshunter/solver/vc/vc_solver.cpp

        src/dshunter/solver/heuristic/greedy.cpp
)

# Create a library for shared components
add_library(core STATIC ${HEADER_IMPLEMENTATIONS})
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Apply build-specific compile definitions ONLY to the core library.
# Using generator expressions supports both single and multi-config generators.
target_compile_definitions(core PRIVATE
        $<$<CONFIG:Testing>:DS_TESTING_MODE>
        $<$<CONFIG:Debug>:DS_TESTING_MODE>
        $<$<CONFIG:Debug>:DS_TRACING_MODE>
)

# Link against necessary libraries (order is important)
target_link_libraries(core PUBLIC
        flow_cutter
        peaty
)

# List of source files for each executable
set(SOURCE_FILES
        src/main.cpp
        src/dshunter/testing/random_graph_gen.cpp
        src/dshunter/testing/small_graph_test.cpp
)

# Define each executable separately
foreach (SRC ${SOURCE_FILES})
    # Get the executable name (based on the source file name without extension)
    get_filename_component(EXEC_NAME ${SRC} NAME_WE)

    add_executable(${EXEC_NAME} ${SRC})
    target_link_libraries(${EXEC_NAME} PRIVATE core)

    # Apply common executable definitions
    target_compile_definitions(${EXEC_NAME} PRIVATE DEBUG_MODE)
endforeach ()

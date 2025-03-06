cmake_minimum_required(VERSION 3.10)

project(SourceCode)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
  
set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -w")

# Include the src directory
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/ext/flow-cutter-pace17)

# Set the executable suffix to .out
set(CMAKE_EXECUTABLE_SUFFIX ".out")

add_library(flow_cutter    
    src/ext/flow-cutter-pace17/src/cell.cpp
    src/ext/flow-cutter-pace17/src/greedy_order.cpp
    src/ext/flow-cutter-pace17/src/list_graph.cpp
    src/ext/flow-cutter-pace17/src/pace.cpp
    src/ext/flow-cutter-pace17/src/tree_decomposition.cpp
)

# List of source files for each executable
set(SOURCE_FILES
    src/bench.cpp
    src/brute.cpp
    src/main.cpp
    src/random_graph_gen.cpp
    src/decomposition_test.cpp
)

set_source_files_properties(
    ${EXTERNAL_LIBRARY_SOURCES}
    PROPERTIES COMPILE_FLAGS -w
)

# Define each executable separately
foreach(SRC ${SOURCE_FILES})
    # Get the executable name (based on the source file name)
    get_filename_component(EXEC_NAME ${SRC} NAME_WE)

    # Create the executable for each source file
    add_executable(${EXEC_NAME} ${SRC})

    target_link_libraries(${EXEC_NAME} PRIVATE flow_cutter)

    # Link against the necessary headers
    target_include_directories(${EXEC_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/ext/flow-cutter-pace17
    )

    target_compile_definitions(${EXEC_NAME} PRIVATE DEBUG_MODE)

    # For the 'bench' target, define DS_BENCHMARK
    if(EXEC_NAME STREQUAL "bench")
        target_compile_definitions(${EXEC_NAME} PRIVATE DS_BENCHMARK)
        target_compile_options(${EXEC_NAME} PRIVATE -O2)
    endif()
        
endforeach()
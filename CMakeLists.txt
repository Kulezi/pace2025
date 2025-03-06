cmake_minimum_required(VERSION 3.10)

project(SourceCode)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
  
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,cppcoreguidelines-*,-clang-diagnostic-c++17-extensions,-cppcoreguidelines-pro-bounds-pointer-arithmetic;-warnings-as-errors=*")
# Include the src directory
include_directories(${CMAKE_SOURCE_DIR}/src)
# Link directories for htd shared library
link_directories(/usr/local/lib)  # Update this with the actual path to the shared library

# Set the executable suffix to .out
set(CMAKE_EXECUTABLE_SUFFIX ".out")

# List of source files for each executable
set(SOURCE_FILES
    src/bench.cpp
    src/brute.cpp
    src/main.cpp
    src/random_graph_gen.cpp
)

# Define each executable separately
foreach(SRC ${SOURCE_FILES})
    # Get the executable name (based on the source file name)
    get_filename_component(EXEC_NAME ${SRC} NAME_WE)

    # Create the executable for each source file
    add_executable(${EXEC_NAME} ${SRC})

    # Link against the necessary headers
    target_include_directories(${EXEC_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

    # For the 'bench' target, define DS_BENCHMARK
    if(EXEC_NAME STREQUAL "bench")
        target_compile_definitions(${EXEC_NAME} PRIVATE DS_BENCHMARK)
        target_compile_options(${EXEC_NAME} PRIVATE -O2)
    endif()
        
endforeach()
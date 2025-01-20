cmake_minimum_required(VERSION 3.10)

project(SourceCode)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

# Include the src directory and the external directory (htd)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(/usr/local/include/htd)  # Path to header files for htd shared library
include_directories(/usr/local/include/htd_io)  # Path to header files for htd shared library
# Link directories for htd shared library
link_directories(/usr/local/lib)  # Update this with the actual path to the shared library

# Set the executable suffix to .out
set(CMAKE_EXECUTABLE_SUFFIX ".out")

# List of source files for each executable
set(SOURCE_FILES
    src/bench.cpp
    src/brute.cpp
    src/main.cpp
    src/out_verifier.cpp
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
        /usr/local/include/htd # Include the htd directory for external dependency
    )

    # Link the external htd shared library
    target_link_libraries(${EXEC_NAME} PRIVATE htd htd_io)  # Link with the shared library 'htd'
endforeach()
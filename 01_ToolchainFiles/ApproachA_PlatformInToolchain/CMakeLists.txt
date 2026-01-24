cmake_minimum_required(VERSION 3.20)

project(ApproachA_Demo C CXX)

# Create a static library
add_library(hello STATIC
    src/hello.c
    src/hello.cpp
)

# Create an executable
add_executable(main src/main.cpp)

# Link the executable with the library
target_link_libraries(main PRIVATE hello)

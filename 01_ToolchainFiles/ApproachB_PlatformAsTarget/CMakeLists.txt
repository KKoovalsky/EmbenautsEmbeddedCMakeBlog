cmake_minimum_required(VERSION 3.20)

project(ApproachB_Demo C CXX)

# Include platform-specific configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/AM243xPlatform.cmake)

# Create a static library using the helper function
am243x_add_library(hello STATIC
    src/hello.c
    src/hello.cpp
)

# Create an executable using the helper function
am243x_add_executable(main src/main.cpp)

# Link the executable with the library
target_link_libraries(main PRIVATE hello)

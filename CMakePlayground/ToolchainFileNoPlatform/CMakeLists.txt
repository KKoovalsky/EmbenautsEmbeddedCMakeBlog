cmake_minimum_required(VERSION 3.20)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type for single-config mode")
set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Build types for multi-config mode")

project(min_toolchain_no_platform_demo C CXX)

add_library(hello STATIC src/hello.c src/hello.cpp)

add_executable(main src/main.cpp)

cmake_minimum_required(VERSION 3.20)
project(mylib C)

if(NOT DEFINED PLATFORM_ID)
    message(FATAL_ERROR "PLATFORM_ID must be set via toolchain file")
endif()

add_library(mylib STATIC mylib.c)

target_include_directories(mylib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Define platform macro for source code (uppercase)
string(TOUPPER "${PLATFORM_ID}" PLATFORM_ID_UPPER)
target_compile_definitions(mylib PRIVATE "PLATFORM_${PLATFORM_ID_UPPER}")

# Set platform property for compatibility checking
set_target_properties(mylib PROPERTIES
    INTERFACE_PLATFORM_ID "${PLATFORM_ID}"
)
set_property(TARGET mylib APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING PLATFORM_ID
)

message(STATUS "Building mylib for platform: ${PLATFORM_ID}")

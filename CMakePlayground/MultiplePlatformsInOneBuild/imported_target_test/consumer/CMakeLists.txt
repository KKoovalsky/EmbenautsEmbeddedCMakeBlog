cmake_minimum_required(VERSION 3.20)
project(consumer C)

if(NOT DEFINED PLATFORM_ID)
    message(FATAL_ERROR "PLATFORM_ID must be set via toolchain file")
endif()

message(STATUS "Consumer targeting platform: ${PLATFORM_ID}")

# Paths to pre-built libraries
set(MYLIB_AM243X_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build_am243x")
set(MYLIB_TMS570_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build_tms570")

# Import pre-built library for AM243x
add_library(mylib_am243x STATIC IMPORTED)
set_target_properties(mylib_am243x PROPERTIES
    IMPORTED_LOCATION "${MYLIB_AM243X_DIR}/libmylib.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../library"
    INTERFACE_PLATFORM_ID "am243x"
)
set_property(TARGET mylib_am243x APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING PLATFORM_ID
)

# Import pre-built library for TMS570
add_library(mylib_tms570 STATIC IMPORTED)
set_target_properties(mylib_tms570 PROPERTIES
    IMPORTED_LOCATION "${MYLIB_TMS570_DIR}/libmylib.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../library"
    INTERFACE_PLATFORM_ID "tms570"
)
set_property(TARGET mylib_tms570 APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING PLATFORM_ID
)

# Create executable
add_executable(consumer main.c)

# Set platform property on executable
set_target_properties(consumer PROPERTIES
    PLATFORM_ID "${PLATFORM_ID}"
)

# Link based on platform
if(PLATFORM_ID STREQUAL "am243x")
    target_link_libraries(consumer PRIVATE mylib_am243x)
elseif(PLATFORM_ID STREQUAL "tms570")
    target_link_libraries(consumer PRIVATE mylib_tms570)
else()
    message(FATAL_ERROR "Unknown PLATFORM_ID: ${PLATFORM_ID}")
endif()

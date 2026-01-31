cmake_minimum_required(VERSION 3.20)

project(MultiplePlatforms_Demo C CXX)

# Include all platform configurations
include(${CMAKE_CURRENT_SOURCE_DIR}/AM243xPlatform.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/MSPM0Platform.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/TMS570Platform.cmake)

# Create libraries for each platform
am243x_add_library(hello_am243x STATIC
    src/hello.c
    src/hello.cpp
)

mspm0_add_library(hello_mspm0 STATIC
    src/hello.c
    src/hello.cpp
)

tms570_add_library(hello_tms570 STATIC
    src/hello.c
    src/hello.cpp
)

# Create executables for each platform
am243x_add_executable(main_am243x src/main.cpp)
target_link_libraries(main_am243x PRIVATE hello_am243x)

mspm0_add_executable(main_mspm0 src/main.cpp)
target_link_libraries(main_mspm0 PRIVATE hello_mspm0)

tms570_add_executable(main_tms570 src/main.cpp)
target_link_libraries(main_tms570 PRIVATE hello_tms570)

# NOTE: The following will cause errors due to platform mismatches - linker detected
# mspm0_add_executable(main_mspm0_ERROR src/main.cpp)
# target_link_libraries(main_mspm0_ERROR PRIVATE hello_am243x)

# NOTE: The following will cause errors due to platform mismatches - CMake detected through interface properties
# tms570_add_executable(main_tms570_ERROR src/main.cpp)
# target_link_libraries(main_tms570_ERROR PRIVATE hello_am243x)

# Test: plain library without platform - will this be caught?
add_library(hello_no_platform STATIC
    src/hello.c
    src/hello.cpp
)

add_library(mylib_am243x STATIC IMPORTED)
set_target_properties(mylib_am243x PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libmylib_am243x.a"
    # INTERFACE_PLATFORM_ID "am243x"
)

add_library(mylib_tms570 STATIC IMPORTED)
set_target_properties(mylib_tms570 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libmylib_tms570.a"
    # INTERFACE_PLATFORM_ID "mspm0"
)

am243x_add_executable(main_am243x_mylib_consumer ./src/mylib_consumer.cpp)
# NOTE: This will work, because we do not set the INTERFACE_PLATFORM_ID on the imported library
target_link_libraries(main_am243x_mylib_consumer PRIVATE mylib_am243x)

# NOTE: This will cause errors due to platform mismatches - linker detected
# mspm0_add_executable(main_mspm0_mylib_consumer ./src/mylib_consumer.cpp)
# target_link_libraries(main_mspm0_mylib_consumer PRIVATE mylib_am243x)

tms570_add_executable(main_tms570_mylib_consumer ./src/mylib_consumer.cpp)
# NOTE: This will work, because we do not set the INTERFACE_PLATFORM_ID on the imported library
target_link_libraries(main_tms570_mylib_consumer PRIVATE mylib_tms570)

# NOTE: Yes, this will cause errors due to platform mismatches - linker detected. Possibly dangerous when the default
#       platform used by tiarmclang is close to the intended platform.
# am243x_add_executable(main_no_platform_test src/main.cpp)
# target_link_libraries(main_no_platform_test PRIVATE hello_no_platform)

# Dump PLATFORM_ID properties for all targets
get_property(ALL_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

message(STATUS "=== Platform ID Properties ===")
foreach(target ${ALL_TARGETS})
    get_target_property(iface_platform_id ${target} INTERFACE_PLATFORM_ID)
    get_target_property(platform_id ${target} PLATFORM_ID)
    message(STATUS "${target}: INTERFACE_PLATFORM_ID='${iface_platform_id}' PLATFORM_ID='${platform_id}'")
endforeach()


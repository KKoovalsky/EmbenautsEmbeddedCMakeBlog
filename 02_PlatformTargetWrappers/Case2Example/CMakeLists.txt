cmake_minimum_required(VERSION 3.20)
project(Case2Example C CXX)

# Include platform and validator
include(cmake/Platform_AM243x.cmake)
include(cmake/ValidatePlatformTargets.cmake)

# Library - uses wrapper (required in Case 2!)
am243x_add_library(mylib src/mylib.c)

# Executable - uses wrapper
am243x_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)

# ============================================================
# Uncomment the following to test the safety net:
# This will fail at configure time because it's not using the wrapper.
# ============================================================
# add_library(naked_lib STATIC src/mylib.c)
# add_executable(naked_app src/main.c)

# STATIC IMPORTED library will work alright, without the wrapper.
add_library(mylib_other_am243x STATIC IMPORTED)
set_target_properties(mylib_other_am243x PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/libmylib_other_am243x.a"
)

am243x_add_executable(mylib_other_consumer src/mylib_other_consumer.cpp)
target_link_libraries(mylib_other_consumer PRIVATE mylib_other_am243x)

# Validate at the end - catches any naked add_library() or add_executable() calls
emb_validate_all_targets_have_platform()

cmake_minimum_required(VERSION 3.20)
project(Case1Example C)

# Include the wrapper and validator
include(cmake/EmbeddedExecutable.cmake)

# A simple library - no wrapper needed in Case 1
# (platform flags come from toolchain file)
add_library(mylib STATIC src/mylib.c)

# Create the firmware executable using the wrapper
embedded_add_executable(firmware
    src/main.c
)

# Link the library
target_link_libraries(firmware PRIVATE mylib)

# ============================================================
# Uncomment the following to test the safety net:
# This will fail at configure time because it's not created with the wrapper.
# ============================================================
# add_executable(test_app src/main.c)

# Validate at the end - catches any naked add_executable() calls
emb_validate_all_executables()

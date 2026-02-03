cmake_minimum_required(VERSION 3.20)
project(Case1Example C)

# Include the wrapper and validator
include(Cmake/EmbeddedExecutable.cmake)
include(Cmake/ValidateLinkerScripts.cmake)

# A simple library - no wrapper needed in Case 1
# (platform flags come from toolchain file)
add_library(mylib STATIC
    Src/main.c  # In real project, this would be separate source files
)

# Create the firmware executable using the wrapper
embedded_add_executable(firmware
    Src/main.c
)

# Link the library
target_link_libraries(firmware PRIVATE mylib)

# ============================================================
# Uncomment the following to test the safety net:
# This will fail at configure time because it's missing
# the linker script (uses add_executable directly)
# ============================================================
# add_executable(test_app Src/main.c)

# Validate at the end - catches any naked add_executable() calls
emb_validate_all_executables_have_linker_script()

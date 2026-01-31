# Approach B: Platform configuration outside the toolchain file
# This toolchain file contains ONLY compiler and cross-compilation setup
# Platform-specific flags are NOT included here

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Toolchain root path - provided by the build context
set(TIARMCLANG_TOOLCHAIN_ROOT "" CACHE PATH
    "Path to the TI ARM Clang toolchain root")

# Forward cache variables to try-compile runs
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
     TIARMCLANG_TOOLCHAIN_ROOT)

# Validate toolchain path
if(TIARMCLANG_TOOLCHAIN_ROOT STREQUAL "")
    message(FATAL_ERROR
        "TIARMCLANG_TOOLCHAIN_ROOT not defined! Set it via -DTIARMCLANG_TOOLCHAIN_ROOT=<path>")
endif()

if(NOT EXISTS ${TIARMCLANG_TOOLCHAIN_ROOT})
    message(FATAL_ERROR
        "TIARMCLANG_TOOLCHAIN_ROOT path '${TIARMCLANG_TOOLCHAIN_ROOT}' does not exist!")
endif()

# Find compiler executable
# tiarmclang handles both C and C++ compilation
find_program(CMAKE_C_COMPILER
  NAMES tiarmclang
  HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
  NO_DEFAULT_PATH
  REQUIRED
)

# Use the same compiler for C++
set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})

# Configure try-compile for bare-metal targets
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Search rules for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# NOTE: No platform-specific flags here!
# Platform configuration is handled via INTERFACE targets in CMakeLists.txt

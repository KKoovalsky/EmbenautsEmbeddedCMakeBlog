# Toolchain file for ARM Cortex-R5 (e.g., AM243x)
# Platform flags are in the toolchain — Approach A from Post #1

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
    message(FATAL_ERROR "TIARMCLANG_TOOLCHAIN_ROOT not defined! Set it via -DTIARMCLANG_TOOLCHAIN_ROOT=<path>")
endif()

if(NOT EXISTS ${TIARMCLANG_TOOLCHAIN_ROOT})
    message(FATAL_ERROR "TIARMCLANG_TOOLCHAIN_ROOT path '${TIARMCLANG_TOOLCHAIN_ROOT}' does not exist!")
endif()

# Find compiler executables
find_program(CMAKE_C_COMPILER
    NAMES tiarmclang
    HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
    NO_DEFAULT_PATH
    REQUIRED
)

# For tiarmclang, C and C++ compilers are the same executable
set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})

# Configure try-compile for bare-metal targets
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Search rules for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Platform-specific flags for Cortex-R5
# These flags are applied to ALL targets in the project
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")

# Common embedded flags
string(APPEND CMAKE_C_FLAGS_INIT " -ffunction-sections -fdata-sections")

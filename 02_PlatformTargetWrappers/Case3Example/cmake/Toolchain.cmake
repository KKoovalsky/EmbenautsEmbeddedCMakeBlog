# Toolchain file for TI ARM Clang - NO platform flags (Approach B)
# Platform flags are defined separately in Platform_*.cmake

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

# Find toolchain utilities
find_program(CMAKE_OBJCOPY
    NAMES tiarmobjcopy
    HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
    NO_DEFAULT_PATH
    REQUIRED
)

find_program(CMAKE_SIZE
    NAMES tiarmsize
    HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
    NO_DEFAULT_PATH
    REQUIRED
)

# Configure try-compile for bare-metal targets
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Search rules for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# NOTE: No platform flags here!
# Platform flags are defined in:
#   - cmake/Platform_AM243x.cmake
#   - cmake/Platform_TMS570.cmake

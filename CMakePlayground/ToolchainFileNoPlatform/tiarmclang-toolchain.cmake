set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    TI_INSTALL_DIR
)

set(TI_INSTALL_DIR "" CACHE STRING "TI libraries and tools installation directory")

if(TI_INSTALL_DIR STREQUAL "")
    message(FATAL_ERROR "TI_INSTALL_DIR not defined! No way to find the compiler")
endif()

if(NOT EXISTS ${TI_INSTALL_DIR})
    message(FATAL_ERROR "TI_INSTALL_DIR path '${TI_INSTALL_DIR}' does not exist!")
endif()

# TI ARM Clang toolchain paths
set(TI_CGT_ARM_LLVM_ROOT "${TI_INSTALL_DIR}/ccs2011/ccs/tools/compiler/ti-cgt-armllvm_4.0.2.LTS")

# Specify the cross compiler
find_program(CMAKE_C_COMPILER NAMES tiarmclang HINTS "${TI_CGT_ARM_LLVM_ROOT}/bin/" REQUIRED)
set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})

# Don't search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Fix try_compile() calls to produce static libraries instead of executables (which require linking)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


# Approach A: Platform Configuration Inside the Toolchain File

This example demonstrates **Approach A** where platform-specific flags are included directly in the toolchain file.

## Structure

- `tiarmclang-cortex-r5.toolchain.cmake` - Toolchain file with Cortex-R5 platform flags included
- `CMakeLists.txt` - Simple project with a library and executable
- `src/` - Source files

## Key Feature

The toolchain file sets `CMAKE_C_FLAGS_INIT`, `CMAKE_CXX_FLAGS_INIT`, and `CMAKE_EXE_LINKER_FLAGS_INIT` with platform-specific flags. **All targets** in the project automatically get these flags.

## Building

```bash

# Configure with the toolchain file
cmake -B build/ -S . \ 
  -DCMAKE_TOOLCHAIN_FILE=$(pwd)/tiarmclang-cortex-r5.toolchain.cmake \
  -DTIARMCLANG_TOOLCHAIN_ROOT=/path/to/ti-cgt-armllvm

# Build
cmake --build build/
```

## What Happens

- Every target (library and executable) is compiled with `-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16`
- It's impossible to forget platform flags - they're applied automatically
- Changing platform requires reconfiguring the build (usually means deleting the build directory)

## Consequences

**Pros:**
- Platform flags are enforced implicitly
- No way to build a target without platform configuration

**Cons:**
- Tight coupling between toolchain and platform
- Experimentation with different platforms requires full rebuilds
- One toolchain file per platform variant

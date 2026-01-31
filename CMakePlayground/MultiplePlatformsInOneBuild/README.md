# Multiple Platforms in One Build

This example demonstrates how Approach B (platform as INTERFACE target) enables building for multiple platforms in a single CMake configuration.

## Structure

- `tiarmclang.toolchain.cmake` - Minimal toolchain file WITHOUT platform flags
- `AM243xPlatform.cmake` - Platform configuration for AM243x (Cortex-R5F, hard float)
- `MSPM0Platform.cmake` - Platform configuration for MSPM0 (Cortex-M0+, soft float)
- `TMS570Platform.cmake` - Platform configuration for TMS570 (Cortex-R4F, hard float)
- `CMakeLists.txt` - Project that builds for all platforms
- `src/` - Shared source files

## Platform Differences

| Platform | CPU | Float ABI | FPU |
|----------|-----|-----------|-----|
| AM243x | Cortex-R5F | hard | vfpv3-d16 |
| TMS570 | Cortex-R4F | hard | vfpv3-d16 |
| MSPM0 | Cortex-M0+ | soft | none |

Three different architectures with different compiler flags, yet they share the same toolchain (tiarmclang).

## What This Demonstrates

With Approach B, you can:
- Include multiple platform configurations in one build
- Create separate libraries and executables for each platform
- Share source files between platforms
- Build everything with a single `cmake --build .` command

## Building

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../tiarmclang.toolchain.cmake \
  -DTIARMCLANG_TOOLCHAIN_ROOT=/path/to/ti-cgt-armllvm
cmake --build .
```

## Output

After building, you get:
- `main_am243x` - Executable built for AM243x (Cortex-R5F)
- `main_tms570` - Executable built for TMS570 (Cortex-R4F)
- `main_mspm0` - Executable built for MSPM0 (Cortex-M0+)
- `libhello_am243x.a` - Library built for AM243x
- `libhello_tms570.a` - Library built for TMS570
- `libhello_mspm0.a` - Library built for MSPM0

## Why This Matters

With Approach A (platform in toolchain), you would need:
- Three separate build directories
- Three separate CMake configurations
- Three separate build commands

With Approach B, one configuration builds everything.

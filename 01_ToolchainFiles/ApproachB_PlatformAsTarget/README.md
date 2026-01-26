# Approach B: Platform Configuration Outside the Toolchain File

This example demonstrates **Approach B** where platform-specific flags are defined as INTERFACE targets in the CMakeLists.txt, NOT in the toolchain file.

## Structure

- `tiarmclang.toolchain.cmake` - Minimal toolchain file WITHOUT platform flags
- `AM243xPlatform.cmake` - Platform-specific configuration with helper functions
- `CMakeLists.txt` - Main project file that includes the platform configuration
- `src/` - Source files

## Key Feature

The toolchain file contains ONLY:
- Compiler selection
- Cross-compilation environment setup
- Search rules

Platform configuration is defined in a separate module file (`AM243xPlatform.cmake`) that:
- Defines platform flags in a variable (`AM243X_PLATFORM_FLAGS`)
- Creates an INTERFACE library (`platform_am243x`) with those flags
- Provides helper functions (`am243x_add_library()`, `am243x_add_executable()`) that automatically link the platform

This modular approach keeps platform-specific code separate and reusable.

## Building

```bash
# Create build directory
mkdir build
cd build

# Configure with the toolchain file
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../tiarmclang.toolchain.cmake \
  -DTIARMCLANG_TOOLCHAIN_ROOT=/path/to/ti-cgt-armllvm

# Build
cmake --build .
```

## What Happens

- The toolchain file sets up the compiler but does NOT apply any platform flags
- Platform flags are stored in the `AM243X_PLATFORM_FLAGS` variable
- `platform_am243x` INTERFACE target is created with these flags
- Helper functions `am243x_add_library()` and `am243x_add_executable()` wrap the standard CMake functions
- These helpers automatically link `platform_am243x` PRIVATE to every target
- The `hello` library and `main` executable use the helper functions, so they get platform flags automatically

## Enforcement via Helper Functions

This example demonstrates **one way to enforce platform binding**: helper functions that wrap `add_library()` and `add_executable()`.

The helper functions (`am243x_add_library()` and `am243x_add_executable()`) automatically link the platform target, making it impossible to forget. To use this approach effectively:

1. **Use only the helper functions** - Never call `add_library()` or `add_executable()` directly
2. **Enforce this rule** - Code reviews or linting should catch direct usage
3. **Name helpers after the platform** - Makes it obvious which platform you're targeting

### The Danger Without Helpers

If you bypass the helpers and use `add_library()` or `add_executable()` directly without linking the platform target, the build will still succeed! The compiler produces a binary without warnings, but it's not built for the intended platform.

This is the failure mode mentioned in the blog post. **Approach B requires explicit enforcement** to prevent this, and helper functions are one practical solution.

## Consequences

**Pros:**
- Multiple platforms can coexist in one build tree
- Platform changes don't require reconfiguring the build
- Explicit and composable platform configuration
- Easy experimentation with different platform variants

**Cons:**
- Forgetting to link against the platform target is possible and dangerous
- Requires discipline or tooling to enforce platform binding
- Less implicit enforcement than Approach A

## When to Use This

This approach works well for:
- Multi-platform projects (same toolchain, different cores)
- Multi-core systems with different platform requirements per core
- Projects where platform experimentation is frequent
- When you want to avoid multiple nearly-identical toolchain files

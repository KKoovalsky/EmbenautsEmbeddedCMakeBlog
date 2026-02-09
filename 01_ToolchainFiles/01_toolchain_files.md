# CMake Toolchain Files in Embedded Projects

## Toolchain, Platform, and Explicit Failure Modes

This post uses **TI ARM Clang** as a concrete example toolchain and **one Cortex-R MCU** as an illustrative platform. The reasoning is not vendor-specific. The same trade-offs apply to any embedded cross toolchain.

The intent is not to argue for a single "correct" structure, but to make **two coherent approaches explicit**, along with their constraints and failure modes.

---

## What a Toolchain File Is Used For

A CMake toolchain file is evaluated very early during configuration. It describes the **build environment**:

* whether the build is native or cross
* which compiler suite is used
* how configure-time checks behave
* how programs, headers, and libraries are located

It does not describe the application, the firmware layout, or the platform logic. When those concerns are mixed, configuration behavior becomes harder to reason about than the code being built.

---

## System Identification and Cross-Compilation State

```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
```

Setting `CMAKE_SYSTEM_NAME` explicitly marks the build as non-hosted. CMake then sets `CMAKE_CROSSCOMPILING = TRUE`.

This variable matters once a project contains both host-side and target-side logic. Without it, CMake scripts cannot reliably distinguish between tools that must run on the host and artifacts that are built only for the target.

---

## Compiler Selection Without Installation Assumptions

The toolchain file selects the compiler family, but should not assume the compiler is installed system-wide or available in `PATH`. That assumption breaks down in CI pipelines and containerized builds.

```cmake
set(TIARMCLANG_TOOLCHAIN_ROOT "" CACHE PATH
    "Path to the TI ARM Clang toolchain root")
```

This cache variable is provided by the higher-level build context—developer environment, CI script, or container setup. The toolchain file does not set a default location. If the path is missing, configuration fails immediately when searching for the compiler.

Compiler executables are resolved explicitly:

```cmake
find_program(CMAKE_C_COMPILER
  NAMES tiarmclang
  HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
  NO_DEFAULT_PATH
  REQUIRED
)

find_program(CMAKE_CXX_COMPILER
  NAMES tiarmclang++
  HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
  NO_DEFAULT_PATH
  REQUIRED
)
```

`NO_DEFAULT_PATH` prevents CMake from searching system paths. If the toolchain is not where you said it would be, configuration fails. No silent fallback to a different compiler.

Cache variables required during try-compile must be explicitly forwarded:

```cmake
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
     TIARMCLANG_TOOLCHAIN_ROOT)
```

Without this, try-compile runs observe a different configuration than the main build. This mismatch is difficult to diagnose once it happens.

---

## Configure-Time Checks on Bare-Metal Targets

CMake validates compilers by building and executing test programs. This does not work for bare-metal targets.

```cmake
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

---

## Search Rules

When cross-compiling, CMake needs to know where to look for different types of files. You have two separate ecosystems: the host (where the build runs) and the target (where the firmware runs).

```cmake
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

These settings control CMake's `find_*` commands:

* **`CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER`** - Never search the target sysroot for executables. Programs like code generators, protocol buffer compilers, or custom build tools run on the host, so CMake should only find them in host paths.

* **`CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY`** - Only search the target sysroot for libraries. Without this, CMake might link against your host's x86 version of a library when you need the ARM version.

* **`CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY`** - Only search the target sysroot for headers. Host headers may have different APIs or assume different architectures.

* **`CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY`** - Only search the target sysroot for CMake package configuration files.

Without these settings, you get build failures that are hard to diagnose. `find_library()` locates your host's x86 version of a library. The linker may accept it during configuration checks, but linking the final firmware binary fails with architecture mismatches. Or worse: it links successfully, but the binary won't run because it was linked against the wrong ABI.

Once you use `find_package()` to locate dependencies, these settings stop being optional. The alternative is debugging why your embedded project decided `/usr/lib` was a reasonable place to find ARM libraries.

---

## Minimal Toolchain File (Toolchain Only)

```cmake
# cmake/toolchains/tiarmclang.toolchain.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TIARMCLANG_TOOLCHAIN_ROOT "" CACHE PATH
    "Path to the TI ARM Clang toolchain root")

list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
     TIARMCLANG_TOOLCHAIN_ROOT)

find_program(CMAKE_C_COMPILER
  NAMES tiarmclang
  HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
  REQUIRED
)

set(CMAKE_CXX_COMPILER ${CMAKE_C_COMPILER})

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

At this point, **no platform is specified**.

---

## The Problem: Building Without Platform Flags

Here's the issue: you can compile and link an executable with this minimal toolchain file. No warnings. No errors. The compiler produces a binary.

What platform is that binary built for? That depends on compiler defaults and internal driver configuration. This is not explicit in the build system, and it should not be treated as a contract.

This matters if you keep platform-specific flags outside the toolchain file. A developer forgets to explicitly link an executable against the platform target. The build succeeds. You flash the device. Then things fail—possibly immediately, possibly intermittently, possibly only under specific conditions.

This is not a theoretical problem. It happens.

There are two coherent approaches to handling platform configuration. Both work. Both have trade-offs. The critical part is understanding which failure modes each approach makes impossible, and which ones require explicit discipline.

---

## Two Structurally Valid Approaches

There are two ways to structure platform configuration. Neither is universally better. Each makes a different class of mistakes impossible.

---

## Approach A: Platform Configuration Inside the Toolchain File

This is the more common approach. If you've worked with embedded CMake builds, you've probably seen this structure.

### Description

The toolchain file specifies:

* compiler
* cross-compilation environment
* **platform-specific architecture flags**

Example (Cortex-R illustration):

```cmake
set(CMAKE_C_FLAGS_INIT
    "-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16")

set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")
```

### When this approach is appropriate

* The project targets **exactly one platform**
* There is no plan to support additional platforms later
* All firmware artifacts share the same architecture
* Reconfiguring the build when flags change is acceptable

### Consequences

* It is impossible to build a binary without platform flags
* Forgetting to bind an executable to a platform cannot happen
* Platform configuration is implicit but enforced

### Failure mode

Platform changes require careful rebuild discipline. Toolchain files are evaluated early, and their values are aggressively cached. Changing architecture flags usually means deleting the build directory and reconfiguring from scratch.

This makes platform experimentation time-consuming. Testing a different CPU variant or FPU configuration requires a full clean → reconfigure → rebuild cycle. For large projects, that can mean several minutes per iteration.

### Full Example

Here's a complete toolchain file implementing Approach A with Cortex-R5 platform flags. The flags used (`-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16`) are an example set that could match TI MCU families like AM263 / AM263P / AM261x, AM64x / AM62x (R5F cores), or TMS570LC43x (Hercules) devices:

```cmake
# cmake/toolchains/tiarmclang-cortex-r5.toolchain.cmake
# Approach A: Platform configuration inside the toolchain file

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

# Platform-specific flags for Cortex-R5
# These flags are applied to ALL targets in the project
set(CMAKE_C_FLAGS_INIT
    "-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16")

set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")

# Linker flags are not needed here - C/C++ flags are forwarded to the linker
```

A complete, runnable example can be found in the `01_ToolchainFiles/ApproachA_PlatformInToolchain` directory of the [EmbenautsEmbeddedCMakeBlog repository](https://github.com/KKoovalsky/EmbenautsEmbeddedCMakeBlog). The example includes a simple project with a static library and executable demonstrating how all targets automatically receive the platform flags. For detailed build instructions, refer to the `README.md` in the example directory.

---

## Approach B: Platform Configuration Outside the Toolchain File

### Description

The toolchain file specifies only:

* compiler
* cross-compilation environment

Platform configuration is applied explicitly via targets.

Example:

```cmake
add_library(platform_cortex_r5 INTERFACE)

target_compile_options(platform_cortex_r5 INTERFACE
  -mcpu=cortex-r5
  -mfloat-abi=hard
  -mfpu=vfpv3-d16
)

target_link_options(platform_cortex_r5 INTERFACE
  -mcpu=cortex-r5
  -mfloat-abi=hard
  -mfpu=vfpv3-d16
)
```

Executables must link against this target.

### When this approach is appropriate

* Multi-platform projects using the same toolchain
* Multi-core systems with different cores
* Independent applications per core
* Little or no shared code between platforms
* Avoiding multiple near-identical toolchain files is desirable

### Consequences

* Platform configuration is explicit and composable
* Multiple platforms can coexist in one build
* Architecture changes propagate through dependencies without reconfiguration

### Failure mode

An executable can be built without platform flags if it is not explicitly bound to a platform target. The build succeeds. The binary may flash successfully. The error appears only at runtime, possibly intermittently.

This approach requires **explicit enforcement** at the build-system level.

### Enforcement Mechanism: Helper Functions

One way to enforce platform binding is to wrap `add_library()` and `add_executable()` with helper functions that automatically link the platform target.

Create a platform-specific module file (e.g., `AM243xPlatform.cmake`):

```cmake
# AM243x Platform Configuration

# Platform-specific flags for AM243x (Cortex-R5F)
set(AM243X_PLATFORM_FLAGS
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

# Define platform configuration as an INTERFACE library
add_library(platform_am243x INTERFACE)

target_compile_options(platform_am243x INTERFACE
  ${AM243X_PLATFORM_FLAGS}
)

target_link_options(platform_am243x INTERFACE
  ${AM243X_PLATFORM_FLAGS}
)

# Helper function to add a library with platform flags automatically linked
function(am243x_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
endfunction()

# Helper function to add an executable with platform flags automatically linked
function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
endfunction()
```

Include this module in your `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyProject C CXX)

# Include platform-specific configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/AM243xPlatform.cmake)

# Use the helper functions instead of add_library/add_executable
am243x_add_library(hello STATIC src/hello.c src/hello.cpp)
am243x_add_executable(main src/main.cpp)
```

With this pattern:

* Every target automatically gets platform flags linked PRIVATE
* The function name makes the target platform explicit
* Forgetting platform flags becomes harder

This still depends on team discipline—code reviews or linting must catch direct usage of `add_library()` or `add_executable()`. It's not bulletproof, but it reduces the failure mode and makes intent explicit.

### Full Example

The toolchain file for Approach B is nearly identical to the minimal toolchain file shown earlier. This is intentional—the toolchain file focuses purely on compiler setup without platform-specific concerns:

```cmake
# cmake/toolchains/tiarmclang.toolchain.cmake
# Approach B: Platform configuration outside the toolchain file

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
# Platform configuration is handled via INTERFACE targets
```

The difference from Approach A: no `CMAKE_C_FLAGS_INIT` or `CMAKE_CXX_FLAGS_INIT`. Platform configuration lives in a separate module.

A complete, runnable example demonstrating Approach B with helper function enforcement can be found in the `01_ToolchainFiles/ApproachB_PlatformAsTarget` directory of the [EmbenautsEmbeddedCMakeBlog repository](https://github.com/KKoovalsky/EmbenautsEmbeddedCMakeBlog).

The example includes:
- The minimal toolchain file shown above
- Platform configuration module (`AM243xPlatform.cmake`) with helper functions
- Demonstration of how targets automatically receive platform flags through the helpers

For detailed build instructions and an explanation of the enforcement mechanism, refer to the `README.md` in the example directory.

---

## The Invalid Middle Ground

A toolchain file without platform flags, combined with platform flags outside the toolchain but no enforcement mechanism, is unsafe.

The compiler produces a binary. The build system does not complain. The error appears only after flashing—possibly immediately, possibly under load, possibly only on certain hardware revisions.

If platform configuration is not in the toolchain file, the build system must make forgetting it structurally difficult or impossible. Helper functions are one approach. Build-time validation is another. The specific mechanism matters less than acknowledging the failure mode exists.

---

## Closing Note

Both approaches work in production systems. The critical part is not which one you choose, but that you understand the failure modes and make them explicit.

A build system should not allow producing artifacts whose intended execution environment is undefined. Whether that enforcement is implicit (Approach A) or requires active discipline (Approach B) depends on your project's constraints and team structure.

---

**License**
© 2026 Kacper Kowalski
This article is licensed under **CC BY-NC-ND 4.0**.

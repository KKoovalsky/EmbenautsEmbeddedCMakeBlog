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

Setting `CMAKE_SYSTEM_NAME` explicitly marks the build as non-hosted. CMake then sets:

```
CMAKE_CROSSCOMPILING = TRUE
```

This variable is essential once a project contains both host-side and target-side logic. It allows CMake scripts to distinguish between:

* tools that must run on the host
* artifacts that are built only for the target

---

## Compiler Selection Without Installation Assumptions

The toolchain file selects the **compiler family**, but must not assume that the compiler is installed system-wide or available in `PATH`. That assumption does not survive CI or containerized builds.

```cmake
set(TIARMCLANG_TOOLCHAIN_ROOT "" CACHE PATH
    "Path to the TI ARM Clang toolchain root")
```

This cache variable is provided by the **higher-level build context** (developer environment, CI, or container setup). The toolchain file does not set a default location. If the path is not provided, configuration will fail when searching for the compiler.

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

`NO_DEFAULT_PATH` prevents searching in system paths. If the toolchain is missing from the specified location, configuration fails immediately. Silent fallback to a different compiler is avoided.

Any **cache variables** that are required during try-compile must be explicitly forwarded:

```cmake
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
     TIARMCLANG_TOOLCHAIN_ROOT)
```

Without this, try-compile runs may observe a different configuration than the main build, which is difficult to diagnose.

---

## Configure-Time Checks on Bare-Metal Targets

CMake validates compilers by building and executing test programs. This does not work for bare-metal targets.

```cmake
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

---

## Search Rules

```cmake
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

This separates:

* host tools
* target headers and libraries

Once `find_package()` is involved, this separation is no longer optional.

---

## Minimal Toolchain File (Toolchain Only)

```cmake
# cmake/toolchains/tiarmclang.toolchain.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TIARMCLANG_TOOLCHAIN_ROOT "" CACHE PATH
    "Path to the TI ARM Clang toolchain root")

find_program(CMAKE_C_COMPILER
  NAMES tiarmclang
  HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
  REQUIRED
)

find_program(CMAKE_CXX_COMPILER
  NAMES tiarmclang++
  HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
  REQUIRED
)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
     TIARMCLANG_TOOLCHAIN_ROOT)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

At this point, **no platform is specified**.

---

## Mind the Gap: No Platform, No Problem?

Here's something that might surprise you: compiling and linking an executable with this minimal toolchain file will work just fine. No warnings. No errors. The compiler happily produces a binary.

But here's the catch: what platform is that binary actually built for? The answer depends on compiler defaults and internal driver configuration—things that are not explicit in your build system and definitely shouldn't be treated as a contract.

This matters because if you decide to keep platform-specific flags outside the toolchain file, you're opening a door to a dangerous failure mode. A developer can forget to explicitly link an executable against the platform target, the build succeeds, you flash it, and then... things break. Or worse, they break intermittently.

This isn't a hypothetical problem. It's a real trap that's easy to fall into.

The good news? This can be overcome. In fact, there are two coherent approaches to handling platform configuration in embedded CMake projects. Both have trade-offs, and we'll explore them in detail. The key is understanding which approach fits your project and making the failure modes explicit.

---

## Two Structurally Valid Approaches

There are two coherent ways to structure platform configuration. Neither is universally better. Each makes a different class of mistakes impossible.

---

## Approach A: Platform Configuration Inside the Toolchain File

This is the most commonly used approach in embedded projects. If you've worked with embedded CMake builds before, you've likely seen this structure.

### Description

The toolchain file specifies:

* compiler
* cross-compilation environment
* **platform-specific architecture flags**

Example (Cortex-R illustration):

```cmake
set(CMAKE_C_FLAGS_INIT
    "-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16")

set(CMAKE_CXX_FLAGS_INIT
    "-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16")
```

### When this approach is appropriate

* The project targets **exactly one platform**
* There is no plan to support additional platforms later
* All firmware artifacts share the same architecture
* It is acceptable to reconfigure the build directory when flags change

### Consequences

* It is impossible to build a binary without platform flags
* Forgetting to bind an executable to a platform cannot happen
* Platform configuration is implicit but enforced

### Failure mode

Platform changes require careful rebuild discipline. Because toolchain files are evaluated early and their values are aggressively cached, changing architecture flags often means you need to invalidate the CMake cache. In practice, this usually means deleting the entire build directory and starting from scratch.

This makes platform experimentation time-consuming. Want to test a different CPU variant or FPU configuration? Be prepared for the full clean → reconfigure → rebuild loop. For large projects, this can mean waiting several minutes just to test a flag change.

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
* Architecture changes propagate naturally through dependencies

### Failure mode

* An executable can be built without platform flags if not explicitly bound
* The build may succeed and produce a flashable binary that is not built for the intended target

This approach requires **explicit enforcement** at the build-system level.

---

## The Invalid Middle Ground

> Toolchain without platform
>
> * platform flags outside the toolchain
> * no enforcement

This configuration is unsafe.

The compiler will produce a binary. The build system will not complain. The error appears only after flashing, or worse, intermittently.

If platform configuration is not in the toolchain, **the build system must make forgetting it impossible**.

---

## Closing Note

Both approaches are used successfully in production systems. The important part is not which one is chosen, but that the failure modes are understood and made explicit.

A build system should not allow producing artifacts whose intended execution environment is undefined.
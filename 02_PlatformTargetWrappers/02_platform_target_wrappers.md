# Platform Target Wrappers in CMake (Embedded)

## Why You Need `add_executable` and `add_library` Wrappers for Embedded Targets

> This post is a direct continuation of **Post #1**.
> Post #1 established two coherent structures:
>
> * **Approach A:** platform flags inside the toolchain file (implicit but enforced)
> * **Approach B:** platform flags outside the toolchain file (explicit but requires enforcement)
>
> This post focuses on the practical wrappers you'll need in either case.

---

## Most of you landed here for Case 1

If you're reading this, you probably have:

* a single platform
* a single linker script
* platform flags already in your toolchain file (Approach A)

And you're wondering: "Do I really need wrappers?"

**Yes — but only for `add_executable`.** At this stage, you don't need to wrap `add_library()`.

Why? Because while the toolchain file handles compile and link *flags*, it doesn't handle:

* **linker scripts** — the memory layout contract
* **map files** — essential for debugging and size analysis
* **output format conversion** — `.elf` to `.bin`, `.hex`, `.srec`
* **post-build steps** — checksums, signing, size reports

These are per-executable concerns. A wrapper centralizes them.

---

## Case 1: Single Platform, Platform in Toolchain File

This is the simplest and often the most appropriate setup.

### What the toolchain file already handles

With Approach A from Post #1, your toolchain file includes platform flags:

```cmake
# In your toolchain file
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")
```

Every target automatically gets these flags. Libraries and executables alike. No wrapper needed for that.

### What the toolchain file doesn't handle

The toolchain file runs once, early, for the whole build. It cannot know:

* which linker script each executable needs
* where to put the map file
* what post-build processing to run

These are **target-specific** concerns.

### The minimal executable wrapper

```cmake
# cmake/EmbeddedExecutable.cmake

function(embedded_add_executable target_name)
    add_executable(${target_name} ${ARGN})

    # Linker script
    target_link_options(${target_name} PRIVATE
        "-Wl,-T${CMAKE_SOURCE_DIR}/linker/device.ld"
    )

    # Map file (named after the target)
    target_link_options(${target_name} PRIVATE
        "-Wl,-Map=$<TARGET_FILE_DIR:${target_name}>/${target_name}.map"
    )
endfunction()
```

Usage:

```cmake
include(cmake/EmbeddedExecutable.cmake)

add_library(mylib STATIC src/mylib.c)  # No wrapper needed

embedded_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)
```

### Why linker scripts belong in a wrapper

A linker script defines the memory layout:

* where code goes (flash regions)
* where data goes (RAM regions)
* stack and heap placement
* interrupt vector table location

Without a linker script, the linker uses defaults that make no sense for your MCU. The binary might link, but it won't run.

Putting the linker script in a wrapper means:

* you cannot forget it
* every executable gets the correct memory layout
* changing the linker script is a one-line change

### Why map files belong in a wrapper

A map file shows:

* final memory usage (flash/RAM)
* symbol addresses
* section sizes
* what got linked and from where

For embedded development, this is not optional debug output. It's how you:

* verify you're not overflowing flash
* debug hard faults (look up addresses)
* optimize size (find the bloat)

Generating it automatically in the wrapper means it's always there when you need it.

### Extended wrapper: output conversion and size report

Most workflows need more than just the ELF:

```cmake
function(embedded_add_executable target_name)
    add_executable(${target_name} ${ARGN})

    # Linker script
    target_link_options(${target_name} PRIVATE
        "-Wl,-T${CMAKE_SOURCE_DIR}/linker/device.ld"
    )

    # Map file
    target_link_options(${target_name} PRIVATE
        "-Wl,-Map=$<TARGET_FILE_DIR:${target_name}>/${target_name}.map"
    )

    # Generate .bin for flashing
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${target_name}>
            $<TARGET_FILE_DIR:${target_name}>/${target_name}.bin
        COMMENT "Generating ${target_name}.bin"
    )

    # Print size summary
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target_name}>
        COMMENT "Size of ${target_name}:"
    )
endfunction()
```

Now every firmware build:

1. Links with the correct memory layout
2. Produces a map file for debugging
3. Generates a flashable binary
4. Shows you the size

### Why you don't need `add_library` wrappers in Case 1

Libraries don't need linker scripts — they're not linked into a final binary by themselves.

Libraries don't need map files — they don't have a memory layout.

With platform flags in the toolchain file, every library already compiles with the correct `-mcpu`, `-mfloat-abi`, etc.

So in Case 1: **wrap executables, leave libraries alone**.

---

## Case 2: Platform Configuration Outside the Toolchain File

When you use Approach B from Post #1, the toolchain file contains no platform flags. This unlocks flexibility (multiple platforms in one build), but requires more discipline.

Now you need wrappers for **both** `add_library` and `add_executable`.

### Why libraries need wrappers now

Without platform flags in the toolchain, a naked `add_library()` compiles with... whatever the compiler defaults to. That's not a contract. That's an accident.

If library A is compiled without `-mfloat-abi=hard` and executable B (which links A) is compiled with it, you get:

* a successful build
* ABI mismatch at runtime
* mysterious crashes, wrong values, silent corruption

The linker often won't catch this. The symptoms appear at runtime.

### The platform target (from Post #1)

```cmake
# cmake/Platform_AM243x.cmake

add_library(platform_am243x INTERFACE)

set(AM243X_FLAGS
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

target_compile_options(platform_am243x INTERFACE ${AM243X_FLAGS})
target_link_options(platform_am243x INTERFACE ${AM243X_FLAGS})
```

### Library wrapper for Approach B

```cmake
function(am243x_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
endfunction()
```

### Executable wrapper for Approach B

```cmake
function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)

    # Linker script
    target_link_options(${target_name} PRIVATE
        "-Wl,-T${CMAKE_SOURCE_DIR}/linker/am243x.ld"
    )

    # Map file
    target_link_options(${target_name} PRIVATE
        "-Wl,-Map=$<TARGET_FILE_DIR:${target_name}>/${target_name}.map"
    )
endfunction()
```

### Usage

```cmake
include(cmake/Platform_AM243x.cmake)

am243x_add_library(mylib STATIC src/mylib.c)
am243x_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)
```

Both the library and executable now have the platform contract.

---

## Case 3: Multiple Platforms, Multiple Linker Scripts

This is where Approach B pays off. You have:

* multiple MCUs (e.g., AM243x and TMS570)
* or multiple memory configurations (flash vs RAM boot)
* or multi-core systems (R5F core 0 vs core 1)

Each combination needs:

* its own platform flags
* its own linker script
* possibly different post-build steps

### Platform targets

```cmake
# cmake/Platform_AM243x.cmake
add_library(platform_am243x INTERFACE)
target_compile_options(platform_am243x INTERFACE
    -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16)
target_link_options(platform_am243x INTERFACE
    -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16)

# cmake/Platform_TMS570.cmake
add_library(platform_tms570 INTERFACE)
target_compile_options(platform_tms570 INTERFACE
    -mcpu=cortex-r4 -mfloat-abi=hard -mfpu=vfpv3-d16)
target_link_options(platform_tms570 INTERFACE
    -mcpu=cortex-r4 -mfloat-abi=hard -mfpu=vfpv3-d16)
```

### Generic wrappers with explicit platform parameter

```cmake
function(platform_add_library target_name)
    cmake_parse_arguments(P "" "PLATFORM" "SOURCES" ${ARGN})

    if(NOT P_PLATFORM)
        message(FATAL_ERROR "platform_add_library: PLATFORM is required")
    endif()

    add_library(${target_name} STATIC ${P_SOURCES})
    target_link_libraries(${target_name} PRIVATE ${P_PLATFORM})
endfunction()

function(platform_add_executable target_name)
    cmake_parse_arguments(P "" "PLATFORM;LINKER_SCRIPT" "SOURCES" ${ARGN})

    if(NOT P_PLATFORM)
        message(FATAL_ERROR "platform_add_executable: PLATFORM is required")
    endif()
    if(NOT P_LINKER_SCRIPT)
        message(FATAL_ERROR "platform_add_executable: LINKER_SCRIPT is required")
    endif()

    add_executable(${target_name} ${P_SOURCES})
    target_link_libraries(${target_name} PRIVATE ${P_PLATFORM})

    target_link_options(${target_name} PRIVATE
        "-Wl,-T${P_LINKER_SCRIPT}"
        "-Wl,-Map=$<TARGET_FILE_DIR:${target_name}>/${target_name}.map"
    )
endfunction()
```

### Usage

```cmake
include(cmake/Platform_AM243x.cmake)
include(cmake/Platform_TMS570.cmake)
include(cmake/PlatformWrappers.cmake)

# Library for AM243x
platform_add_library(mylib_am243x
    PLATFORM platform_am243x
    SOURCES src/mylib.c
)

# Library for TMS570
platform_add_library(mylib_tms570
    PLATFORM platform_tms570
    SOURCES src/mylib.c
)

# Executables
platform_add_executable(firmware_am243x
    PLATFORM platform_am243x
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/am243x.ld
    SOURCES src/main.c
)

platform_add_executable(firmware_tms570
    PLATFORM platform_tms570
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/tms570.ld
    SOURCES src/main.c
)

target_link_libraries(firmware_am243x PRIVATE mylib_am243x)
target_link_libraries(firmware_tms570 PRIVATE mylib_tms570)
```

### The enforcement benefit

With `PLATFORM` as a required parameter:

* forgetting the platform is a configure-time error
* the platform is visible in the CMakeLists.txt (reviewable)
* mixing platforms accidentally becomes harder

---

## Summary: When to Use Which Pattern

| Scenario | Library wrapper? | Executable wrapper? |
|----------|------------------|---------------------|
| Case 1: Single platform in toolchain | No | Yes (linker script, map, post-build) |
| Case 2: Platform outside toolchain | Yes (platform flags) | Yes (platform + linker script + map) |
| Case 3: Multiple platforms | Yes (explicit platform) | Yes (explicit platform + linker script) |

---

## Closing Note

Whether you use Approach A or Approach B from Post #1, you'll end up wanting an `add_executable` wrapper. Linker scripts, map files, and post-build steps are per-executable concerns that don't belong scattered across CMakeLists files.

The difference is whether you also need `add_library` wrappers — and that depends entirely on where your platform flags live.

Start with Case 1 if it fits your project. Move to Case 2 or 3 when you actually need the flexibility. Premature generalization in build systems creates maintenance burden without payoff.

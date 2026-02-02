# Platform Target Wrappers in CMake (Embedded)

## Why You Need `add_executable` and `add_library` Wrappers for Embedded Targets

> This post is a direct continuation of **Post #1**.
> Post #1 established two coherent structures:
>
> * **Approach A:** platform flags inside the toolchain file (implicit but enforced)
> * **Approach B:** platform flags outside the toolchain file (explicit but requires enforcement)
>
> This post focuses on creating **minimal wrappers** that produce a binary runnable on your device, and on **safety mechanisms** to avoid common mistakes.
>
> At the end of the post, we'll extend the minimal wrapper with useful extras: map files, binary conversion, size reports.

---

## Breaking the "one project = one executable" mindset

Many embedded projects treat CMake like an IDE project file: one build produces one firmware binary. That's it.

This mindset is inherited from IDE-based workflows (Code Composer Studio, Keil, IAR) where creating a new executable means creating a new project, duplicating configuration, and maintaining parallel build setups.

CMake doesn't have this limitation. A single CMake project can produce:

* the main firmware
* hardware abstraction layer (HAL) unit tests
* peripheral driver test executables
* hardware validation tests (run on real hardware, test specific functionality)
* integration test binaries
* bootloader variants
* factory test firmware
* diagnostic tools
* example applications for each peripheral
* benchmark executables

Without wrappers, each of these would require copy-pasting linker script paths, platform flags, and post-build steps. With wrappers, adding a new executable is one line.

This is why wrappers matter: they make multiple executables practical.

---

## Most of you landed here for Case 1

If you're reading this, you probably have:

* a single platform
* a single linker script
* platform flags already in your toolchain file (Approach A)

And you're wondering: "Do I really need wrappers?"

**Yes — but only for `add_executable()`.** At this stage, you don't need to wrap `add_library()`.

Why? Because while the toolchain file handles compile and link *flags*, it doesn't handle **linker scripts** — the memory layout contract that makes your binary actually run on the device.

This is a per-executable concern. A wrapper centralizes it.

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

### The linker script as an INTERFACE target

The linker script deserves its own target. Why?

1. **Dependency tracking** — CMake doesn't automatically relink when a linker script changes. You need to tell it.
2. **Reusability** — multiple executables can link the same linker script target
3. **Encapsulation** — linker flags stay with the linker script, not scattered in wrappers

```cmake
# cmake/DeviceLinkerScript.cmake

set(DEVICE_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker/device.ld")

add_library(device_linker_script INTERFACE)

target_link_options(device_linker_script INTERFACE
    "-Wl,-T${DEVICE_LINKER_SCRIPT}"
)

# Critical: relink when linker script changes
set_property(TARGET device_linker_script APPEND PROPERTY
    INTERFACE_LINK_DEPENDS "${DEVICE_LINKER_SCRIPT}"
)
```

The `INTERFACE_LINK_DEPENDS` property is the key. Without it, modifying the linker script does nothing — CMake thinks the executable is up to date. With it, any change to the `.ld` file triggers a relink.

### The executable wrapper

The wrapper links the linker script target automatically:

```cmake
# cmake/EmbeddedExecutable.cmake

include(${CMAKE_CURRENT_LIST_DIR}/DeviceLinkerScript.cmake)

function(embedded_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE device_linker_script)

    # Mark this executable as having a proper linker script
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_LINKER_SCRIPT TRUE)
endfunction()
```

That's the minimum. The linker script is applied, and changes to it trigger a relink.

### The safety net: catching naked executables

What if someone uses `add_executable()` directly, bypassing the wrapper? The build succeeds, but the binary uses the compiler's default linker script — which almost certainly doesn't match your device's memory layout.

Add a validation function that runs at the end of configuration:

```cmake
# cmake/ValidateLinkerScripts.cmake

function(emb_validate_all_executables_have_linker_script)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "EXECUTABLE")
            continue()
        endif()

        get_target_property(has_linker_script ${target} EMB_HAS_LINKER_SCRIPT)
        if(NOT has_linker_script)
            message(FATAL_ERROR
                "Executable '${target}' does not have a linker script.\n"
                "Use embedded_add_executable() instead of add_executable().\n"
                "Without a linker script, the binary will use compiler defaults "
                "and likely won't run on your device.")
        endif()
    endforeach()
endfunction()
```

Call it at the end of your root `CMakeLists.txt`:

```cmake
# CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(MyFirmware C)

include(cmake/EmbeddedExecutable.cmake)
include(cmake/ValidateLinkerScripts.cmake)

add_library(mylib STATIC src/mylib.c)

embedded_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)

# Validate at the end — catches any naked add_executable() calls
emb_validate_all_executables_have_linker_script()
```

Now if someone adds:

```cmake
add_executable(test_app src/test.c)  # Forgot the wrapper!
```

Configuration fails immediately:

```
CMake Error at cmake/ValidateLinkerScripts.cmake:15 (message):
  Executable 'test_app' does not have a linker script.
  Use embedded_add_executable() instead of add_executable().
  Without a linker script, the binary will use compiler defaults and likely
  won't run on your device.
```

No silent failures. No binaries with wrong memory layouts.

### Usage

```cmake
include(cmake/EmbeddedExecutable.cmake)

# Libraries don't need wrappers
add_library(mylib STATIC src/mylib.c)

# Executables get the linker script automatically
embedded_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)
```

### Why this structure matters

**Without `INTERFACE_LINK_DEPENDS`:**

```
$ vim linker/device.ld   # change memory regions
$ make
$ # nothing happens — CMake thinks firmware is up to date
$ # you flash the old binary
$ # you debug for an hour wondering why your changes didn't work
```

**With `INTERFACE_LINK_DEPENDS`:**

```
$ vim linker/device.ld
$ make
[1/1] Linking CXX executable firmware.elf
```

The relink happens automatically. No stale binaries.

### Why linker scripts belong in a target

A linker script defines the memory layout:

* where code goes (flash regions)
* where data goes (RAM regions)
* stack and heap placement
* interrupt vector table location

Without a linker script, the linker uses defaults that make no sense for your MCU. The binary might link, but it won't run.

Modeling the linker script as an INTERFACE target means:

* you cannot forget it (the wrapper links it automatically)
* every executable gets the correct memory layout
* changing the linker script triggers a relink
* multiple executables share the same linker script target

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

Without platform flags in the toolchain, a naked `add_library()` compiles with whatever the compiler defaults to. That's not a contract. That's an accident.

If library A is compiled without `-mfloat-abi=hard` and executable B (which links A) is compiled with it, the linker will *most probably* detect the mismatch. But "most probably" is not "always". For example, ARM Cortex-R4 and Cortex-R5 have compatible ABIs — mixing them may go unnoticed at link time. The binary might even work at runtime. But "might work" is not a foundation for reliable firmware.

> **Note on library types:** This post focuses on `STATIC` libraries for simplicity. In embedded projects, static libraries cover the vast majority of use cases. If you need `SHARED`, `OBJECT`, or `MODULE` libraries, extend the wrapper to parse the library type argument — but you'll rarely need to.
>
> `INTERFACE` libraries are safe without wrappers — they don't compile any sources, so there's nothing to miscompile.

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
    add_library(${target_name} STATIC ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)

    # Mark this library as having platform flags
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()
```

### Linker script target for AM243x

Same pattern as Case 1 — the linker script gets its own target with `INTERFACE_LINK_DEPENDS`:

```cmake
# cmake/Platform_AM243x.cmake (continued)

set(AM243X_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker/am243x.ld")

add_library(linker_am243x INTERFACE)
target_link_options(linker_am243x INTERFACE "-Wl,-T${AM243X_LINKER_SCRIPT}")
set_property(TARGET linker_am243x APPEND PROPERTY
    INTERFACE_LINK_DEPENDS "${AM243X_LINKER_SCRIPT}")
```

### Executable wrapper for Approach B

```cmake
function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x linker_am243x)

    # Safety net
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_LINKER_SCRIPT TRUE)
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()
```

### The extended safety net: validating libraries too

In Case 2, we must validate both executables and libraries. A naked `add_library()` without platform flags is just as dangerous as a naked `add_executable()` without a linker script.

```cmake
# cmake/ValidatePlatformTargets.cmake

function(emb_validate_all_targets_have_platform)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)

        # Check executables for linker script
        if(target_type STREQUAL "EXECUTABLE")
            get_target_property(has_linker_script ${target} EMB_HAS_LINKER_SCRIPT)
            if(NOT has_linker_script)
                message(FATAL_ERROR
                    "Executable '${target}' does not have a linker script.\n"
                    "Use am243x_add_executable() instead of add_executable().")
            endif()
        endif()

        # Check libraries for platform flags (STATIC, SHARED, OBJECT, MODULE)
        if(target_type MATCHES "^(STATIC_LIBRARY|SHARED_LIBRARY|OBJECT_LIBRARY|MODULE_LIBRARY)$")
            get_target_property(has_platform ${target} EMB_HAS_PLATFORM)
            if(NOT has_platform)
                message(FATAL_ERROR
                    "Library '${target}' (${target_type}) does not have platform flags.\n"
                    "Use am243x_add_library() instead of add_library().\n"
                    "Without platform flags, the library may have ABI mismatches.")
            endif()
        endif()

        # INTERFACE libraries are safe — they don't compile sources
    endforeach()
endfunction()
```

### Usage

```cmake
include(cmake/Platform_AM243x.cmake)
include(cmake/ValidatePlatformTargets.cmake)

am243x_add_library(mylib src/mylib.c)
am243x_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)

# Validate at the end — catches naked add_library() and add_executable() calls
emb_validate_all_targets_have_platform()
```

Both the library and executable now have the platform contract, and the validator ensures nothing slips through.

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

### Platform and linker script targets

Each platform module defines both the platform flags and the linker script target:

```cmake
# cmake/Platform_AM243x.cmake
add_library(platform_am243x INTERFACE)
target_compile_options(platform_am243x INTERFACE
    -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16)
target_link_options(platform_am243x INTERFACE
    -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16)

set(AM243X_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker/am243x.ld")
add_library(linker_am243x INTERFACE)
target_link_options(linker_am243x INTERFACE "-Wl,-T${AM243X_LINKER_SCRIPT}")
set_property(TARGET linker_am243x APPEND PROPERTY
    INTERFACE_LINK_DEPENDS "${AM243X_LINKER_SCRIPT}")
```

```cmake
# cmake/Platform_TMS570.cmake
add_library(platform_tms570 INTERFACE)
target_compile_options(platform_tms570 INTERFACE
    -mcpu=cortex-r4 -mfloat-abi=hard -mfpu=vfpv3-d16)
target_link_options(platform_tms570 INTERFACE
    -mcpu=cortex-r4 -mfloat-abi=hard -mfpu=vfpv3-d16)

set(TMS570_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker/tms570.ld")
add_library(linker_tms570 INTERFACE)
target_link_options(linker_tms570 INTERFACE "-Wl,-T${TMS570_LINKER_SCRIPT}")
set_property(TARGET linker_tms570 APPEND PROPERTY
    INTERFACE_LINK_DEPENDS "${TMS570_LINKER_SCRIPT}")
```

### Platform-specific wrappers

With hardcoded linker scripts per platform, each platform gets its own wrappers:

```cmake
# cmake/Platform_AM243x.cmake (continued)

function(am243x_add_library target_name)
    add_library(${target_name} STATIC ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()

function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x linker_am243x)
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_LINKER_SCRIPT TRUE)
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()
```

```cmake
# cmake/Platform_TMS570.cmake (continued)

function(tms570_add_library target_name)
    add_library(${target_name} STATIC ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_tms570)
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()

function(tms570_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_tms570 linker_tms570)
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_LINKER_SCRIPT TRUE)
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()
```

The `emb_validate_all_targets_have_platform()` function catches any target missing the properties, regardless of which platform it should have used.

### Usage

```cmake
include(cmake/Platform_AM243x.cmake)
include(cmake/Platform_TMS570.cmake)
include(cmake/ValidatePlatformTargets.cmake)

# Libraries
am243x_add_library(mylib_am243x src/mylib.c)
tms570_add_library(mylib_tms570 src/mylib.c)

# Executables
am243x_add_executable(firmware_am243x src/main.c)
tms570_add_executable(firmware_tms570 src/main.c)

target_link_libraries(firmware_am243x PRIVATE mylib_am243x)
target_link_libraries(firmware_tms570 PRIVATE mylib_tms570)

# Validate all targets
emb_validate_all_targets_have_platform()
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
| Case 1: Single platform in toolchain | No | Yes (linker script) |
| Case 2: Platform outside toolchain | Yes (platform flags) | Yes (platform + linker script) |
| Case 3: Multiple platforms | Yes (per-platform flags) | Yes (per-platform flags + linker script) |

---

## Closing Note

Whether you use Approach A or Approach B from Post #1, you'll end up wanting an `add_executable` wrapper. The linker script is a per-executable concern that doesn't belong scattered across CMakeLists files.

The difference is whether you also need `add_library` wrappers — and that depends entirely on where your platform flags live.

Start with Case 1 if it fits your project. Move to Case 2 or 3 when you actually need the flexibility. Premature generalization in build systems creates maintenance burden without payoff.

---

## Addendum: Extending the Minimal Wrapper

The wrappers above are intentionally minimal — just enough to produce a runnable binary. In practice, you'll want more.

### Map files

A map file shows memory usage, symbol addresses, and section sizes. Essential for debugging hard faults and tracking flash/RAM consumption:

```cmake
function(embedded_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE device_linker_script)

    # Add map file generation
    target_link_options(${target_name} PRIVATE
        "-Wl,-Map=$<TARGET_FILE_DIR:${target_name}>/${target_name}.map"
    )
endfunction()
```

### Binary conversion

Most flash tools want `.bin` or `.hex`, not ELF:

```cmake
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${target_name}>
            $<TARGET_FILE_DIR:${target_name}>/${target_name}.bin
        COMMENT "Generating ${target_name}.bin"
    )
```

### Size report

Print flash/RAM usage after every build:

```cmake
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target_name}>
        COMMENT "Size of ${target_name}:"
    )
```

These extras will be covered in detail in a future post.

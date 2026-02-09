---
title: "Platform Target Wrappers in CMake (Embedded)"
description: "Creating minimal add_executable and add_library wrappers for embedded targets, with safety mechanisms and useful extras."
date: 2025-01-20
---

# CMake for Embedded: Why add_executable() Needs a Platform Contract

## How wrappers and validation turn conventions into compile-time guarantees

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
* exploratory tests - when you need to poke at hardware behavior
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

# TI linker uses different flag syntax
target_link_options(device_linker_script INTERFACE
    "-Wl,${DEVICE_LINKER_SCRIPT}"
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

    # Mark this executable as targeting embedded
    set_property(TARGET ${target_name} PROPERTY EMB_IS_EXECUTABLE TRUE)
endfunction()
```

That's the minimum. The linker script is applied, and changes to it trigger a relink.

> **Note:** This wrapper is intentionally minimal. See the [Addendum](#addendum-extending-the-minimal-wrapper) at the end for useful extras: map files, binary conversion, size reports.

### The safety net: catching naked executables

What if someone uses `add_executable()` directly, bypassing the wrapper? The build succeeds, but the binary uses the compiler's default linker script — which almost certainly doesn't match your device's memory layout.

Add a validation function (in the same file as the wrapper):

```cmake
# cmake/EmbeddedExecutable.cmake (continued)

function(emb_validate_all_executables)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "EXECUTABLE")
            continue()
        endif()

        get_target_property(is_emb_executable ${target} EMB_IS_EXECUTABLE)
        if(NOT is_emb_executable)
            message(FATAL_ERROR
                "Executable '${target}' is not targeting embedded.\n"
                "Use embedded_add_executable() instead of add_executable().")
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

add_library(mylib STATIC src/mylib.c)

embedded_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)

# Validate at the end — catches any naked add_executable() calls
emb_validate_all_executables()
```

Now if someone adds:

```cmake
add_executable(test_app src/test.c)  # Forgot the wrapper!
```

Configuration fails immediately:

```
CMake Error at cmake/EmbeddedExecutable.cmake:22 (message):
  Executable 'test_app' is not targeting embedded.
  Use embedded_add_executable() instead of add_executable().
```

No silent failures. No binaries with wrong memory layouts.

> **Note:** We don't need a separate validator for the linker script here — the wrapper handles everything in one function. In Case 3, where we split executable creation from linker script linking, we'll need two separate validators.

### Usage

```cmake
include(cmake/EmbeddedExecutable.cmake)

# Libraries don't need wrappers
add_library(mylib STATIC src/mylib.c)

# Executables get the linker script automatically
embedded_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE mylib)

# add_subdirectory() calls, add more executables etc. ...

# Validate at the end of the root CMakeLists.txt
emb_validate_all_executables()
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


### Why you don't need `add_library` wrappers in Case 1

Libraries don't need linker scripts — they're not linked into a final binary by themselves.

Libraries don't need map files — they don't have a memory layout.

With platform flags in the toolchain file, every library already compiles with the correct `-mcpu`, `-mfloat-abi`, etc.

So in Case 1: **wrap executables, leave libraries alone**.

> **Example:** See [`Case1Example/`](Case1Example/) for a complete working example.

---

## Case 2: Platform Configuration Outside the Toolchain File

When you use Approach B from Post #1, the toolchain file contains no platform flags. This unlocks flexibility (multiple platforms in one build), but requires more discipline.

Now you need wrappers for **both** `add_library` and `add_executable`.

### Why libraries need wrappers now

Without platform flags in the toolchain, a naked `add_library()` compiles with whatever the compiler defaults to. That's not a contract. That's an accident.

If library A is compiled without `-mfloat-abi=hard` and executable B (which links A) is compiled with it, the linker will *most probably* detect the mismatch. But "most probably" is not "always". For example, ARM Cortex-R4 and Cortex-R5 have compatible ABIs — mixing them may go unnoticed at link time. The binary might even work at runtime. But "might work" is not a foundation for reliable firmware.

> **Note on library types:** This post focuses on `STATIC` libraries for simplicity. In embedded projects, static libraries cover the majority of use cases. If you need `SHARED`, `OBJECT`, or `MODULE` libraries, extend the wrapper to parse the library type argument.
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
target_link_options(linker_am243x INTERFACE "-Wl,${AM243X_LINKER_SCRIPT}")
set_property(TARGET linker_am243x APPEND PROPERTY
    INTERFACE_LINK_DEPENDS "${AM243X_LINKER_SCRIPT}")
```

### Executable wrapper for Approach B

```cmake
function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x linker_am243x)

    # Safety net
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

        if(target_type MATCHES "^(EXECUTABLE|STATIC_LIBRARY|SHARED_LIBRARY|OBJECT_LIBRARY|MODULE_LIBRARY)$")
            get_target_property(has_platform ${target} EMB_HAS_PLATFORM)
            if(NOT has_platform)
                message(FATAL_ERROR
                    "Target '${target}' (${target_type}) does not have platform flags.\n"
                    "Use am243x_add_*() wrappers instead of bare add_library() or add_executable()\n"
                    "Without platform flags, the library may have ABI mismatches.")
            endif()
        endif()

        # INTERFACE libraries are safe — they don't compile sources
    endforeach()
endfunction()
```

> **Note on IMPORTED targets:** With a single platform, there's no need to validate IMPORTED targets — they can only link against one platform anyway. The developer adding an IMPORTED library is responsible for ensuring it was cross-compiled for the correct platform. In Case 3 (multiple platforms), we'll add validation to catch platform mismatches.

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

> **Need multiple linker scripts per platform?** See Case 3, which shows how to separate executable creation from linker script linking, with compile-time safety to prevent mismatches.

> **Example:** See [`Case2Example/`](Case2Example/) for a complete working example.

---

## Case 3: Multiple Platforms, Multiple Linker Scripts

This is where Approach B pays off. You have:

* multiple MCUs (e.g., AM243x and TMS570)
* multiple memory configurations per MCU (internal RAM only, with external RAM)
* possibly multi-core systems (R5F core 0 vs core 1)

Each combination needs its own platform flags and linker script. With 2 platforms × 2 linker scripts each, hardcoding combinations in wrapper names leads to combinatorial explosion. Instead, we separate executable creation from linker script linking.

### Platform targets (the long way)

First, let's see what a platform target looks like expanded:

```cmake
# cmake/Platform_AM243x.cmake

set(AM243X_FLAGS
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

add_library(platform_am243x INTERFACE)
target_compile_options(platform_am243x INTERFACE ${AM243X_FLAGS})
target_link_options(platform_am243x INTERFACE ${AM243X_FLAGS})

# Mark the platform for compatibility checking
set_property(TARGET platform_am243x PROPERTY
    INTERFACE_EMB_PLATFORM "AM243x")
set_property(TARGET platform_am243x APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
```

```cmake
# cmake/Platform_TMS570.cmake

set(TMS570_FLAGS
    -mcpu=cortex-r4
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

add_library(platform_tms570 INTERFACE)
target_compile_options(platform_tms570 INTERFACE ${TMS570_FLAGS})
target_link_options(platform_tms570 INTERFACE ${TMS570_FLAGS})

set_property(TARGET platform_tms570 PROPERTY
    INTERFACE_EMB_PLATFORM "TMS570")
set_property(TARGET platform_tms570 APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
```

That's verbose. We'll create a helper function shortly, but first let's understand the key mechanism.

#### How `COMPATIBLE_INTERFACE_STRING` works

When you set `COMPATIBLE_INTERFACE_STRING` on a target, you're telling CMake: "this property must have the same value across all linked targets."

Here's what happens:

1. **Property propagation:** When target A links target B, CMake looks at B's `INTERFACE_EMB_PLATFORM` property and propagates it to A.

2. **Consistency check:** If A links multiple targets (B and C), CMake verifies that all `INTERFACE_EMB_PLATFORM` values match. If B says "AM243x" and C says "TMS570", CMake fails at generation time.

3. **The "head" target wins:** The linking target (executable or library doing the linking) can also set `EMB_PLATFORM` directly. If it does, all dependencies must agree with that value.

This is a built-in CMake mechanism — no custom validation code needed. The check happens automatically during the generation phase, before any compilation starts.

Other compatible interface properties:
- `COMPATIBLE_INTERFACE_BOOL` — all values must be the same boolean
- `COMPATIBLE_INTERFACE_NUMBER_MIN` — take the minimum value
- `COMPATIBLE_INTERFACE_NUMBER_MAX` — take the maximum value

We use `STRING` because platform names are strings that must match exactly.

**Important caveat:** If a target doesn't set the property at all, it silently passes the compatibility check — CMake only compares targets that *have* the property. This is why we still need a separate validator: to ensure all relevant targets actually have `EMB_PLATFORM` set in the first place.

### Platform helper function

Instead of repeating the platform setup for each MCU, use a helper function:

```cmake
# cmake/EmbeddedPlatform.cmake

function(emb_add_platform target_name platform_id)
    add_library(${target_name} INTERFACE)

    # Remaining arguments are compiler/linker flags
    set(flags ${ARGN})
    target_compile_options(${target_name} INTERFACE ${flags})
    target_link_options(${target_name} INTERFACE ${flags})

    # Tag with platform for compatibility checking
    set_property(TARGET ${target_name} PROPERTY
        INTERFACE_EMB_PLATFORM "${platform_id}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()
```

Now platform definitions become one-liners:

```cmake
# cmake/Platform_AM243x.cmake
include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedPlatform.cmake)

emb_add_platform(platform_am243x "AM243x"
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)
```

```cmake
# cmake/Platform_TMS570.cmake
include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedPlatform.cmake)

emb_add_platform(platform_tms570 "TMS570"
    -mcpu=cortex-r4
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)
```

### Linker script targets

A helper function creates linker script targets with the correct platform tag:

```cmake
# cmake/EmbeddedLinkerScript.cmake

function(emb_add_linker_script target_name platform_name linker_script_path)
    add_library(${target_name} INTERFACE)
    target_link_options(${target_name} INTERFACE "-Wl,${linker_script_path}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        INTERFACE_LINK_DEPENDS "${linker_script_path}")

    # Mark as linker script (for validation)
    set_property(TARGET ${target_name} PROPERTY EMB_IS_LINKER_SCRIPT TRUE)

    # Tag with platform for compatibility checking
    set_property(TARGET ${target_name} PROPERTY
        INTERFACE_EMB_PLATFORM "${platform_name}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()
```

Each platform defines its linker scripts:

```cmake
# cmake/Platform_AM243x.cmake (continued)

emb_add_linker_script(linker_am243x_internal
    "AM243x"
    "${CMAKE_SOURCE_DIR}/linker/am243x_internal.ld")

emb_add_linker_script(linker_am243x_external
    "AM243x"
    "${CMAKE_SOURCE_DIR}/linker/am243x_external.ld")
```

```cmake
# cmake/Platform_TMS570.cmake (continued)

emb_add_linker_script(linker_tms570_internal
    "TMS570"
    "${CMAKE_SOURCE_DIR}/linker/tms570_internal.ld")

emb_add_linker_script(linker_tms570_external
    "TMS570"
    "${CMAKE_SOURCE_DIR}/linker/tms570_external.ld")
```

### Generic target wrappers

Instead of duplicating wrapper logic per platform, create generic wrappers that take the platform target as a parameter:

```cmake
# cmake/EmbeddedTargets.cmake

function(emb_add_library platform_target target_name)
    add_library(${target_name} STATIC ${ARGN})
    target_link_libraries(${target_name} PRIVATE ${platform_target})

    # Get platform ID from the platform target
    get_target_property(platform_id ${platform_target} INTERFACE_EMB_PLATFORM)

    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
    set_property(TARGET ${target_name} PROPERTY EMB_PLATFORM "${platform_id}")
    # INTERFACE_EMB_PLATFORM is needed for consumers to see the platform ID
    set_property(TARGET ${target_name} PROPERTY INTERFACE_EMB_PLATFORM "${platform_id}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()

function(emb_add_executable platform_target target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE ${platform_target})

    get_target_property(platform_id ${platform_target} INTERFACE_EMB_PLATFORM)

    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
    set_property(TARGET ${target_name} PROPERTY EMB_PLATFORM "${platform_id}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()
```

### Platform-specific aliases

Each platform file creates convenient aliases:

```cmake
# cmake/Platform_AM243x.cmake (continued)

macro(am243x_add_library target_name)
    emb_add_library(platform_am243x ${target_name} ${ARGN})
endmacro()

macro(am243x_add_executable target_name)
    emb_add_executable(platform_am243x ${target_name} ${ARGN})
endmacro()
```

```cmake
# cmake/Platform_TMS570.cmake (continued)

macro(tms570_add_library target_name)
    emb_add_library(platform_tms570 ${target_name} ${ARGN})
endmacro()

macro(tms570_add_executable target_name)
    emb_add_executable(platform_tms570 ${target_name} ${ARGN})
endmacro()
```

Now adding a new platform is just: define the platform target, define the linker scripts, create two one-line macros.

### Linking linker scripts

No special wrapper needed — use `target_link_libraries()` directly:

```cmake
am243x_add_executable(firmware src/main.c)
target_link_libraries(firmware PRIVATE linker_am243x_internal)
```

If you try to link a TMS570 linker script to an AM243x executable, CMake fails at generation time:

```
CMake Error: Property EMB_PLATFORM on target "firmware" does not match the
INTERFACE_EMB_PLATFORM property requirement of dependency "linker_tms570_internal".
```

### Validation: two separate checks

Split the validation into two functions: one for platform flags, one for linker scripts.

**Platform validator** — checks that all libraries and executables have platform flags:

```cmake
# cmake/ValidatePlatform.cmake

function(emb_validate_all_targets_have_platform)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(importedTargets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)

    foreach(target IN LISTS targets) importedTargets
        get_target_property(target_type ${target} TYPE)

        # Check compiled targets (STATIC, SHARED, OBJECT, MODULE, EXECUTABLE)
        if(target_type MATCHES "^(STATIC_LIBRARY|SHARED_LIBRARY|OBJECT_LIBRARY|MODULE_LIBRARY|EXECUTABLE)$")
            get_target_property(has_platform ${target} EMB_HAS_PLATFORM)
            if(NOT has_platform)
                message(FATAL_ERROR
                    "Target '${target}' (${target_type}) does not have platform flags.\n"
                    "Use <platform>_add_library() or <platform>_add_executable().")
            endif()
        endif()

    endforeach()
endfunction()
```

**Linker script validator** — checks that all executables link a linker script target:

```cmake
# cmake/ValidateLinkerScript.cmake

function(emb_validate_all_executables_have_linker_script)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "EXECUTABLE")
            continue()
        endif()

        # Check if any directly linked target is a linker script
        # NOTE: In case you link the linker script indirectly, you would need to traverse the indirect dependencies
        #       against INTERFACE dependencies, to check if the linker script is linked.
        get_target_property(linked_libs ${target} LINK_LIBRARIES)
        set(has_linker_script FALSE)

        foreach(lib IN LISTS linked_libs)
            if(TARGET ${lib})
                get_target_property(is_linker_script ${lib} EMB_IS_LINKER_SCRIPT)
                if(is_linker_script)
                    set(has_linker_script TRUE)
                    break()
                endif()
            endif()
        endforeach()

        if(NOT has_linker_script)
            message(FATAL_ERROR
                "Executable '${target}' does not link a linker script target.\n"
                "Add: target_link_libraries(${target} PRIVATE <linker_script_target>)")
        endif()
    endforeach()
endfunction()
```

> **Note on IMPORTED targets:** The platform validator checks that IMPORTED libraries have `EMB_HAS_PLATFORM` set. This is a useful safety check ensuring IMPORTED targets are linked only with their intended platform. You can create a wrapper for adding IMPORTED libraries that sets this property, or set it manually:
>
> ```cmake
> add_library(vendor_lib STATIC IMPORTED)
> set_target_properties(vendor_lib PROPERTIES
>     IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/vendor/libvendor.a"
>     EMB_HAS_PLATFORM TRUE
>     INTERFACE_EMB_PLATFORM "AM243x")
> set_property(TARGET vendor_lib APPEND PROPERTY
>     COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
> ```
>
> Either way, it documents which platform the IMPORTED library targets.

### Usage

```cmake
include(cmake/EmbeddedLinkerScript.cmake)
include(cmake/Platform_AM243x.cmake)
include(cmake/Platform_TMS570.cmake)
include(cmake/ValidatePlatform.cmake)
include(cmake/ValidateLinkerScript.cmake)

# Libraries
am243x_add_library(mylib_am243x src/mylib.c)
tms570_add_library(mylib_tms570 src/mylib.c)

# Executables — link linker script with target_link_libraries()
am243x_add_executable(firmware_am243x_int src/main.c)
target_link_libraries(firmware_am243x_int PRIVATE linker_am243x_internal)

am243x_add_executable(firmware_am243x_ext src/main.c)
target_link_libraries(firmware_am243x_ext PRIVATE linker_am243x_external)

tms570_add_executable(firmware_tms570 src/main.c)
target_link_libraries(firmware_tms570 PRIVATE linker_tms570_internal)

target_link_libraries(firmware_am243x_int PRIVATE mylib_am243x)
target_link_libraries(firmware_am243x_ext PRIVATE mylib_am243x)
target_link_libraries(firmware_tms570 PRIVATE mylib_tms570)

# This would fail at generation time (platform mismatch):
# target_link_libraries(firmware_am243x_int PRIVATE linker_tms570_internal)

# Validate all targets
emb_validate_all_targets_have_platform()
emb_validate_all_executables_have_linker_script()
```

### The enforcement layers

This pattern provides three layers of safety:

1. **Platform flags** — wrappers ensure every target compiles with correct flags
2. **Platform/linker compatibility** — `COMPATIBLE_INTERFACE_STRING` catches mismatches at generation time
3. **Linker script presence** — validation catches forgotten `target_link_libraries()` calls

> **Example:** See [`Case3Example/`](Case3Example/) for a complete working example.

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

### Finding toolchain utilities

CMake doesn't automatically find `objcopy` and `size` for cross toolchains. Add these to your toolchain file:

```cmake
# In your toolchain file (e.g., Toolchain.cmake)

# For TI ARM Clang
find_program(CMAKE_OBJCOPY
    NAMES tiarmobjcopy
    HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
    REQUIRED
)

find_program(CMAKE_SIZE
    NAMES tiarmsize
    HINTS "${TIARMCLANG_TOOLCHAIN_ROOT}/bin"
    REQUIRED
)

# For GNU ARM toolchain, use:
# find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy ...)
# find_program(CMAKE_SIZE NAMES arm-none-eabi-size ...)
```

### Map files

A map file shows memory usage, symbol addresses, and section sizes. Essential for debugging hard faults and tracking flash/RAM consumption:

```cmake
function(embedded_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE device_linker_script)

    # Add map file generation (TI linker syntax)
    target_link_options(${target_name} PRIVATE
        "-Wl,-m=$<TARGET_FILE_DIR:${target_name}>/${target_name}.map"
    )
endfunction()
```

### Binary conversion

Most flash tools want `.bin` or `.hex`, not ELF.

**`.bin`** — raw binary, no metadata, smallest size:

```cmake
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${target_name}>
            $<TARGET_FILE_DIR:${target_name}>/${target_name}.bin
        COMMENT "Generating ${target_name}.bin"
    )
```

**`.hex`** — Intel HEX format, includes addresses and checksums:

```cmake
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:${target_name}>
            $<TARGET_FILE_DIR:${target_name}>/${target_name}.hex
        COMMENT "Generating ${target_name}.hex"
    )
```

Use `.bin` when the load address is known externally (bootloaders, OTA). Use `.hex` when the flash tool needs address information or you have non-contiguous memory regions.

### Size report

Print flash/RAM usage after every build:

```cmake
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target_name}>
        COMMENT "Size of ${target_name}:"
    )
```

> **Example:** All these extras are implemented in [`Case3Example/`](Case3Example/).

---

**License**
© 2026 Kacper Kowalski
This article is licensed under **CC BY-NC-ND 4.0**.

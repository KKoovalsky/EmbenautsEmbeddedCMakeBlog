# Custom Transitive Properties Demo (CMake 3.30+)

This example demonstrates how CMake's Custom Transitive Properties can detect
indirect linker script linking.

## The Problem

In embedded projects, we want to ensure every executable links a linker script.
But what if the linker script is linked indirectly?

```
my_app -> app_lib -> intermediate_lib -> linker_script_target
```

A configure-time validator using `get_target_property()` won't see the
transitive dependency.

## The Solution: TRANSITIVE_LINK_PROPERTIES

CMake 3.30+ introduced `TRANSITIVE_LINK_PROPERTIES` which allows custom
properties to propagate through the dependency chain.

```cmake
add_library(linker_script_a INTERFACE)
set_target_properties(linker_script_a PROPERTIES
    TRANSITIVE_LINK_PROPERTIES "EMB_HAS_LINKER_SCRIPT"
    INTERFACE_EMB_HAS_LINKER_SCRIPT "linker_script_a"
)
```

## Results

### Configure time (get_target_property)
```
linker_script_a INTERFACE_EMB_HAS_LINKER_SCRIPT: linker_script_a
intermediate_lib EMB_HAS_LINKER_SCRIPT: NOTFOUND
my_app EMB_HAS_LINKER_SCRIPT: NOTFOUND
```

### Generation time ($<TARGET_PROPERTY:...>)
```
Target: intermediate_lib
EMB_HAS_LINKER_SCRIPT: linker_script_a

Target: my_app
EMB_HAS_LINKER_SCRIPT: linker_script_a
```

The property propagates transitively at generation time!

## Approaches

1. **TRANSITIVE_LINK_PROPERTIES** - Custom property propagates through deps.
   Check via generator expressions (generation/build time).

2. **COMPATIBLE_INTERFACE_STRING** - CMake fails at generation time if
   linked targets have conflicting values. Good for platform mismatch detection.

## Usage

```bash
cmake -B build
cmake --build build
cat build/linker_script_check_*.txt
```

## Limitation

Configure-time validators (`get_target_property()`) won't see transitive
values. You need generator expressions, which evaluate at generation time.

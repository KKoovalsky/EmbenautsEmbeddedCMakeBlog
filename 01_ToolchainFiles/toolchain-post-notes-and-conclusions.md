
# CMake Toolchain Files in Embedded Projects  
## Notes, Conclusions, and Design Rationale

This document summarizes the full reasoning, observations, trade-offs, and conclusions that led to the final blog post about CMake toolchain files in embedded projects.

It is not intended for publication.  
Its purpose is to preserve *why* certain decisions were made and *which failure modes* were considered.

---

## Initial Motivation

The original question was whether platform-specific configuration (CPU, ABI, FPU flags) should live in:
- the CMake toolchain file, or
- project-level CMake targets

The discussion quickly moved beyond style preferences into **failure modes**, **scalability**, and **developer error prevention**.

---

## Key Observations

### 1. Toolchain files are evaluated early and cached

- Toolchain files are processed before most project logic.
- Values introduced via toolchain files are cached aggressively.
- Changing architecture flags placed in the toolchain often requires deleting the build directory.
- This behavior is easy to forget and difficult to reason about in larger projects.

---

### 2. A minimal toolchain still builds *something*

A minimal toolchain file that selects the compiler but does not specify platform flags still successfully compiles and links binaries.

The compiler emits no error or warning.

This means:
- a binary is produced
- the target platform is implicit or undefined
- this is *not* a meaningful or safe contract

---

### 3. Separating platform from toolchain introduces a real risk

When platform-specific flags are moved out of the toolchain and into targets:
- a developer can forget to bind an executable to a platform
- the build still succeeds
- the binary may fail only at runtime

This is a silent failure mode and must be addressed explicitly.

---

## Two Valid Structural Approaches

There is no universally correct solution.  
Instead, two coherent approaches exist.

### Approach A: Platform configuration in the toolchain file

- Platform flags are implicit and enforced.
- Impossible to forget platform configuration.
- Reconfiguration is often required when flags change.

### Approach B: Platform configuration outside the toolchain file

- Platform flags are explicit and composable.
- Multiple platforms can coexist.
- Requires enforcement to avoid silent misconfiguration.

---

## Invalid Middle Ground

Toolchain without platform  
+ platform outside toolchain  
+ no enforcement  

This configuration is unsafe.

---

## Toolchain Discovery and Reproducibility

- Toolchains must not be assumed to be system-wide.
- Paths must be provided by the higher-level build context.
- Toolchain variables must be forwarded via CMAKE_TRY_COMPILE_PLATFORM_VARIABLES.

---

## Outcome

The final post:
- presents both approaches symmetrically
- avoids recommendations
- makes failure modes explicit
- is suitable for a blog post or book chapter

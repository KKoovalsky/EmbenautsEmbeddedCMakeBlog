# Platform_TMS570.cmake
# TMS570 platform (Cortex-R4) with multiple linker scripts

include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedPlatform.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedLinkerScript.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedTargets.cmake)

# Platform target
emb_add_platform(platform_tms570 "TMS570"
    -mcpu=cortex-r4
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

# Linker scripts - internal RAM only vs with external RAM
emb_add_linker_script(linker_tms570_internal
    "TMS570"
    "${CMAKE_SOURCE_DIR}/linker/tms570_internal.ld")

emb_add_linker_script(linker_tms570_external
    "TMS570"
    "${CMAKE_SOURCE_DIR}/linker/tms570_external.ld")

# Platform-specific convenience macros
macro(tms570_add_library target_name)
    emb_add_library(platform_tms570 ${target_name} ${ARGN})
endmacro()

macro(tms570_add_executable target_name)
    emb_add_executable(platform_tms570 ${target_name} ${ARGN})
endmacro()

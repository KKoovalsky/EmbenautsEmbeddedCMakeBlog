# Platform_AM243x.cmake
# AM243x platform (Cortex-R5) with multiple linker scripts

include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedPlatform.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedLinkerScript.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/EmbeddedTargets.cmake)

# Platform target
emb_add_platform(platform_am243x "AM243x"
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

# Linker scripts - internal RAM only vs with external RAM
emb_add_linker_script(linker_am243x_internal
    "AM243x"
    "${CMAKE_SOURCE_DIR}/linker/am243x_internal.ld")

emb_add_linker_script(linker_am243x_external
    "AM243x"
    "${CMAKE_SOURCE_DIR}/linker/am243x_external.ld")

# Platform-specific convenience macros
macro(am243x_add_library target_name)
    emb_add_library(platform_am243x ${target_name} ${ARGN})
endmacro()

macro(am243x_add_executable target_name)
    emb_add_executable(platform_am243x ${target_name} ${ARGN})
endmacro()

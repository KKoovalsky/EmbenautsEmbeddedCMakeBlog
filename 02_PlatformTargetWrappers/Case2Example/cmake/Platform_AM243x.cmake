# Platform_AM243x.cmake
# Platform flags and linker script for AM243x (Cortex-R5)

# Platform flags as INTERFACE target
add_library(platform_am243x INTERFACE)

set(AM243X_FLAGS
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

target_compile_options(platform_am243x INTERFACE ${AM243X_FLAGS})
target_link_options(platform_am243x INTERFACE ${AM243X_FLAGS})

# Linker script as INTERFACE target
set(AM243X_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/linker/am243x.ld")

add_library(linker_am243x INTERFACE)
target_link_options(linker_am243x INTERFACE "-Wl,${AM243X_LINKER_SCRIPT}")
set_property(TARGET linker_am243x APPEND PROPERTY
    INTERFACE_LINK_DEPENDS "${AM243X_LINKER_SCRIPT}")

# Library wrapper
function(am243x_add_library target_name)
    add_library(${target_name} STATIC ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)

    # Mark this library as having platform flags
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()

# Executable wrapper
function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x linker_am243x)

    # Safety net
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
endfunction()

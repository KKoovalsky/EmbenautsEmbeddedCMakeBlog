# MSPM0 Platform Configuration
# Cortex-M0+ (no FPU, soft float)

set(MSPM0_PLATFORM_FLAGS
    -mcpu=cortex-m0plus
    -mthumb
    -mfloat-abi=soft
)

add_library(platform_mspm0 INTERFACE)

target_compile_options(platform_mspm0 INTERFACE
  ${MSPM0_PLATFORM_FLAGS}
)

target_link_options(platform_mspm0 INTERFACE
  ${MSPM0_PLATFORM_FLAGS}
)

function(mspm0_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_mspm0)
    set_property(TARGET ${target_name} PROPERTY INTERFACE_PLATFORM_ID "mspm0")
    set_property(TARGET ${target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING PLATFORM_ID)
endfunction()

function(mspm0_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_mspm0)
    set_property(TARGET ${target_name} PROPERTY PLATFORM_ID "mspm0")
    set_property(TARGET ${target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING PLATFORM_ID)
endfunction()

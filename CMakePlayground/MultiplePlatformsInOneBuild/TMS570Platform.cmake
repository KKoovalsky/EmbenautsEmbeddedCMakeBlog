# TMS570 Platform Configuration
# Cortex-R4F (hard float, vfpv3-d16)

set(TMS570_PLATFORM_FLAGS
    -mcpu=cortex-r4
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

add_library(platform_tms570 INTERFACE)

target_compile_options(platform_tms570 INTERFACE
  ${TMS570_PLATFORM_FLAGS}
)

target_link_options(platform_tms570 INTERFACE
  ${TMS570_PLATFORM_FLAGS}
)

function(tms570_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_tms570)
    set_property(TARGET ${target_name} PROPERTY INTERFACE_PLATFORM_ID "tms570")
    set_property(TARGET ${target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING PLATFORM_ID)
endfunction()

function(tms570_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_tms570)
    set_property(TARGET ${target_name} PROPERTY PLATFORM_ID "tms570")
    set_property(TARGET ${target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING PLATFORM_ID)
endfunction()

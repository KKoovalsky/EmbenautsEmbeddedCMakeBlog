# AM243x Platform Configuration
# Cortex-R5F @ 800 MHz

set(AM243X_PLATFORM_FLAGS
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

add_library(platform_am243x INTERFACE)

target_compile_options(platform_am243x INTERFACE
  ${AM243X_PLATFORM_FLAGS}
)

target_link_options(platform_am243x INTERFACE
  ${AM243X_PLATFORM_FLAGS}
)

function(am243x_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
    # Propagate platform ID for compatibility checking (but not compile flags)
    set_property(TARGET ${target_name} PROPERTY INTERFACE_PLATFORM_ID "am243x")
    set_property(TARGET ${target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING PLATFORM_ID)
endfunction()

function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
    # Set platform ID for compatibility checking
    set_property(TARGET ${target_name} PROPERTY PLATFORM_ID "am243x")
    set_property(TARGET ${target_name} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING PLATFORM_ID)
endfunction()

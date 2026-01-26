# AM243x Platform Configuration
# Defines platform flags and helper functions for AM243x (Cortex-R5F) targets

# Platform-specific flags for AM243x (Cortex-R5F)
set(AM243X_PLATFORM_FLAGS
    -mcpu=cortex-r5
    -mfloat-abi=hard
    -mfpu=vfpv3-d16
)

# Define platform configuration as an INTERFACE library
add_library(platform_am243x INTERFACE)

target_compile_options(platform_am243x INTERFACE
  ${AM243X_PLATFORM_FLAGS}
)

target_link_options(platform_am243x INTERFACE
  ${AM243X_PLATFORM_FLAGS}
)

# Helper function to add a library with platform flags automatically linked
function(am243x_add_library target_name)
    add_library(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
endfunction()

# Helper function to add an executable with platform flags automatically linked
function(am243x_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE platform_am243x)
endfunction()

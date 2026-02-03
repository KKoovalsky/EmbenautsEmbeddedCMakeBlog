# DeviceLinkerScript.cmake
# Creates an INTERFACE target for the device linker script

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

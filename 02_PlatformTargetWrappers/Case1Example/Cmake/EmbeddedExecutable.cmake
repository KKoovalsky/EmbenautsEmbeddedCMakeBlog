# EmbeddedExecutable.cmake
# Wrapper for add_executable that applies linker script automatically

include(${CMAKE_CURRENT_LIST_DIR}/DeviceLinkerScript.cmake)

function(embedded_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE device_linker_script)

    # Mark this executable as having a proper linker script
    set_property(TARGET ${target_name} PROPERTY EMB_HAS_LINKER_SCRIPT TRUE)
endfunction()

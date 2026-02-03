# EmbeddedExecutable.cmake
# Wrapper for add_executable that applies linker script automatically

include(${CMAKE_CURRENT_LIST_DIR}/DeviceLinkerScript.cmake)

function(embedded_add_executable target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE device_linker_script)

    # Mark this executable as targeting embedded
    set_property(TARGET ${target_name} PROPERTY EMB_IS_EXECUTABLE TRUE)
endfunction()

function(emb_validate_all_executables)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "EXECUTABLE")
            continue()
        endif()

        get_target_property(is_emb_executable ${target} EMB_IS_EXECUTABLE)
        if(NOT is_emb_executable)
            message(FATAL_ERROR
                "Executable '${target}' is not targeting embedded.\n"
                "Use embedded_add_executable() instead of add_executable().\n")
        endif()
    endforeach()
endfunction()

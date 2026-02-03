# ValidateLinkerScripts.cmake
# Validates that all executables have a linker script

function(emb_validate_all_executables_have_linker_script)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "EXECUTABLE")
            continue()
        endif()

        get_target_property(has_linker_script ${target} EMB_HAS_LINKER_SCRIPT)
        if(NOT has_linker_script)
            message(FATAL_ERROR
                "Executable '${target}' does not have a linker script.\n"
                "Use embedded_add_executable() instead of add_executable().\n"
                "Without a linker script, the binary will use compiler defaults "
                "and likely won't run on your device.")
        endif()
    endforeach()
endfunction()

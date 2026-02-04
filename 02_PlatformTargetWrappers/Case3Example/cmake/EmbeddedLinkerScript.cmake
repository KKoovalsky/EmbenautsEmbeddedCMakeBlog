# EmbeddedLinkerScript.cmake
# Helper function to create linker script INTERFACE targets

function(emb_add_linker_script target_name platform_name linker_script_path)
    add_library(${target_name} INTERFACE)
    target_link_options(${target_name} INTERFACE "-Wl,${linker_script_path}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        INTERFACE_LINK_DEPENDS "${linker_script_path}")

    # Mark as linker script (for validation)
    set_property(TARGET ${target_name} PROPERTY EMB_IS_LINKER_SCRIPT TRUE)

    # Tag with platform for compatibility checking
    set_property(TARGET ${target_name} PROPERTY
        INTERFACE_EMB_PLATFORM "${platform_name}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()

function(emb_validate_all_executables_have_linker_script)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "EXECUTABLE")
            continue()
        endif()

        # Check if any directly linked target is a linker script
        # NOTE: In case you link the linker script indirectly, you would need to traverse the indirect dependencies
        #       against INTERFACE dependencies, to check if the linker script is linked.
        get_target_property(linked_libs ${target} LINK_LIBRARIES)
        set(has_linker_script FALSE)

        foreach(lib IN LISTS linked_libs)
            if(TARGET ${lib})
                get_target_property(is_linker_script ${lib} EMB_IS_LINKER_SCRIPT)
                if(is_linker_script)
                    set(has_linker_script TRUE)
                    break()
                endif()
            endif()
        endforeach()

        if(NOT has_linker_script)
            message(FATAL_ERROR
                "Executable '${target}' does not link a linker script target.\n"
                "Add: target_link_libraries(${target} PRIVATE <linker_script_target>)")
        endif()
    endforeach()
endfunction()

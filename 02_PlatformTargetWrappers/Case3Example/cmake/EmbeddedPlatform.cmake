# EmbeddedPlatform.cmake
# Helper function to create platform INTERFACE targets

function(emb_add_platform target_name platform_id)
    add_library(${target_name} INTERFACE)

    # Remaining arguments are compiler/linker flags
    set(flags ${ARGN})
    target_compile_options(${target_name} INTERFACE ${flags})
    target_link_options(${target_name} INTERFACE ${flags})

    # Tag with platform for compatibility checking
    set_property(TARGET ${target_name} PROPERTY
        INTERFACE_EMB_PLATFORM "${platform_id}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()

function(emb_validate_all_targets_have_platform)
    get_property(targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(imported_targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)

    foreach(target IN LISTS targets imported_targets)
        get_target_property(target_type ${target} TYPE)

        # Check compiled targets (STATIC, SHARED, OBJECT, MODULE, EXECUTABLE)
        if(target_type MATCHES "^(STATIC_LIBRARY|SHARED_LIBRARY|OBJECT_LIBRARY|MODULE_LIBRARY|EXECUTABLE)$")
            get_target_property(has_platform ${target} EMB_HAS_PLATFORM)
            if(NOT has_platform)
                message(FATAL_ERROR
                    "Target '${target}' (${target_type}) does not have platform flags.\n"
                    "Use <platform>_add_library() or <platform>_add_executable().")
            endif()
        endif()
    endforeach()
endfunction()

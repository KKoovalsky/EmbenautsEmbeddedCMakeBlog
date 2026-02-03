# ValidatePlatformTargets.cmake
# Validates that all compiled targets have platform flags

function(emb_validate_all_targets_have_platform)
    get_property(targets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        get_target_property(target_type ${target} TYPE)

        if(target_type MATCHES "^(EXECUTABLE|STATIC_LIBRARY|SHARED_LIBRARY|OBJECT_LIBRARY|MODULE_LIBRARY)$")
            get_target_property(has_platform ${target} EMB_HAS_PLATFORM)
            if(NOT has_platform)
                message(FATAL_ERROR
                    "Target '${target}' (${target_type}) does not have platform flags.\n"
                    "Use am243x_add_*() wrappers instead of bare add_library() or add_executable().\n"
                    "Without platform flags, the target may have ABI mismatches.")
            endif()
        endif()

        # INTERFACE libraries are safe — they don't compile sources
    endforeach()
endfunction()

# EmbeddedTargets.cmake
# Generic target wrappers that take platform as a parameter

function(emb_add_library platform_target target_name)
    add_library(${target_name} STATIC ${ARGN})
    target_link_libraries(${target_name} PRIVATE ${platform_target})

    # Get platform ID from the platform target
    get_target_property(platform_id ${platform_target} INTERFACE_EMB_PLATFORM)

    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
    set_property(TARGET ${target_name} PROPERTY EMB_PLATFORM "${platform_id}")
    # INTERFACE_EMB_PLATFORM is needed for consumers to see the platform ID
    set_property(TARGET ${target_name} PROPERTY INTERFACE_EMB_PLATFORM "${platform_id}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()

function(emb_add_executable platform_target target_name)
    add_executable(${target_name} ${ARGN})
    target_link_libraries(${target_name} PRIVATE ${platform_target})

    get_target_property(platform_id ${platform_target} INTERFACE_EMB_PLATFORM)

    set_property(TARGET ${target_name} PROPERTY EMB_HAS_PLATFORM TRUE)
    set_property(TARGET ${target_name} PROPERTY EMB_PLATFORM "${platform_id}")
    set_property(TARGET ${target_name} APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
endfunction()

cmake_minimum_required(VERSION 3.30)
project(CustomTransitivePropertiesDemo C)

# =============================================================================
# This example demonstrates CMake's Custom Transitive Properties feature
# (introduced in CMake 3.30) for detecting indirect linker script linking.
#
# Scenario:
#   executable -> intermediate_lib -> linker_script_target
#
# We want to detect at generation time that the executable has a linker script
# linked, even though it's indirect.
# =============================================================================

# -----------------------------------------------------------------------------
# Approach 1: Using TRANSITIVE_LINK_PROPERTIES
# -----------------------------------------------------------------------------

# Simulate a linker script INTERFACE target
add_library(linker_script_a INTERFACE)

# Mark it as a linker script using custom transitive property
set_target_properties(linker_script_a PROPERTIES
    # Declare that EMB_HAS_LINKER_SCRIPT should propagate transitively
    TRANSITIVE_LINK_PROPERTIES "EMB_HAS_LINKER_SCRIPT"
    # Set the INTERFACE_ version so it propagates to dependents
    INTERFACE_EMB_HAS_LINKER_SCRIPT "linker_script_a"
)

# An intermediate library that links the linker script
add_library(intermediate_lib STATIC intermediate.c)
target_link_libraries(intermediate_lib PRIVATE linker_script_a)

# The executable links the intermediate library (NOT the linker script directly)
add_library(app_lib STATIC app_lib.c)
target_link_libraries(app_lib PRIVATE intermediate_lib)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE app_lib)

# -----------------------------------------------------------------------------
# Check the property using generator expression
# This runs at GENERATION time, not configure time
# -----------------------------------------------------------------------------

# Method 1: Print via file(GENERATE)
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/linker_script_check_$<TARGET_PROPERTY:my_app,NAME>.txt"
    CONTENT "Target: my_app\nEMB_HAS_LINKER_SCRIPT: $<TARGET_PROPERTY:my_app,EMB_HAS_LINKER_SCRIPT>\n"
)

file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/linker_script_check_$<TARGET_PROPERTY:intermediate_lib,NAME>.txt"
    CONTENT "Target: intermediate_lib\nEMB_HAS_LINKER_SCRIPT: $<TARGET_PROPERTY:intermediate_lib,EMB_HAS_LINKER_SCRIPT>\n"
)

# Method 2: Use in a custom command (e.g., to fail the build if missing)
add_custom_target(check_linker_script ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Checking linker script for my_app..."
    COMMAND ${CMAKE_COMMAND} -E echo "EMB_HAS_LINKER_SCRIPT = $<TARGET_PROPERTY:my_app,EMB_HAS_LINKER_SCRIPT>"
    COMMENT "Verifying linker script presence"
)

# -----------------------------------------------------------------------------
# Approach 2: Using COMPATIBLE_INTERFACE_STRING (alternative)
# This makes CMake fail at generation time if values don't match
# -----------------------------------------------------------------------------

add_library(linker_script_b INTERFACE)
set_target_properties(linker_script_b PROPERTIES
    INTERFACE_EMB_LINKER_SCRIPT_NAME "linker_script_b"
)
# Tell CMake this property must be consistent across all linked targets
set_property(TARGET linker_script_b APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING EMB_LINKER_SCRIPT_NAME)

add_library(another_lib STATIC another.c)
target_link_libraries(another_lib PRIVATE linker_script_b)

add_executable(another_app another_main.c)
target_link_libraries(another_app PRIVATE another_lib)

# If we tried to also link a different linker script with a different name,
# CMake would fail at generation time:
# add_library(linker_script_c INTERFACE)
# set_target_properties(linker_script_c PROPERTIES
#     INTERFACE_EMB_LINKER_SCRIPT_NAME "linker_script_c"  # Different name!
# )
# set_property(TARGET linker_script_c APPEND PROPERTY
#     COMPATIBLE_INTERFACE_STRING EMB_LINKER_SCRIPT_NAME)
# target_link_libraries(another_app PRIVATE linker_script_c)  # Would fail!

# -----------------------------------------------------------------------------
# Print summary at configure time (these won't show transitive values!)
# -----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== Configure-time property check (won't see transitive values) ===")

get_target_property(val linker_script_a INTERFACE_EMB_HAS_LINKER_SCRIPT)
message(STATUS "linker_script_a INTERFACE_EMB_HAS_LINKER_SCRIPT: ${val}")

get_target_property(val intermediate_lib EMB_HAS_LINKER_SCRIPT)
message(STATUS "intermediate_lib EMB_HAS_LINKER_SCRIPT: ${val}")

get_target_property(val my_app EMB_HAS_LINKER_SCRIPT)
message(STATUS "my_app EMB_HAS_LINKER_SCRIPT: ${val}")

message(STATUS "")
message(STATUS "Note: Configure-time get_target_property() doesn't see transitive values.")
message(STATUS "Check the generated files in build dir for generation-time values.")
message(STATUS "")

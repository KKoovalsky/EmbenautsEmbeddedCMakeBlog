cmake_minimum_required(VERSION 3.20)
project(Case3Example C)

# Include platform configurations and validators
include(cmake/Platform_AM243x.cmake)
include(cmake/Platform_TMS570.cmake)

# =============================================================================
# Libraries - one per platform
# =============================================================================

am243x_add_library(mylib_am243x src/mylib.c)
tms570_add_library(mylib_tms570 src/mylib.c)

# =============================================================================
# Executables - multiple per platform (different linker scripts)
# =============================================================================

### ===========================================================================
# POSITIVE TEST CASES
### ===========================================================================

# AM243x with internal RAM linker script
am243x_add_executable(firmware_am243x_int src/main.c)
target_link_libraries(firmware_am243x_int PRIVATE mylib_am243x linker_am243x_internal)

# AM243x with external RAM linker script
am243x_add_executable(firmware_am243x_ext src/main.c)
target_link_libraries(firmware_am243x_ext PRIVATE mylib_am243x linker_am243x_external)

# TMS570 with internal RAM linker script
tms570_add_executable(firmware_tms570_int src/main.c)
target_link_libraries(firmware_tms570_int PRIVATE mylib_tms570 linker_tms570_internal)

# TMS570 with external RAM linker script
tms570_add_executable(firmware_tms570_ext src/main.c)
target_link_libraries(firmware_tms570_ext PRIVATE mylib_tms570 linker_tms570_external)

# IMPORTED library properly configured
add_library(vendor_am243x STATIC IMPORTED)
set_target_properties(vendor_am243x PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libvendor_am243x.a"
    EMB_HAS_PLATFORM TRUE
    INTERFACE_EMB_PLATFORM "AM243x"
)
set_property(TARGET vendor_am243x APPEND PROPERTY
    COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)

am243x_add_executable(firmware_with_vendor src/main.c)
target_link_libraries(firmware_with_vendor PRIVATE mylib_am243x vendor_am243x linker_am243x_internal)

# =============================================================================
# Uncomment to test safety mechanisms:
# =============================================================================

### =========================================================================== 
# NEGATIVE TEST CASES
### =========================================================================== 
# --- Test 1: Platform mismatch on linker script (COMPATIBLE_INTERFACE_STRING catches this) ---
# am243x_add_executable(test1_mismatch src/main.c)
# target_link_libraries(test1_mismatch PRIVATE linker_tms570_internal mylib_am243x)

# --- Test 2: Platform mismatch on exe and library (COMPATIBLE_INTERFACE_STRING catches this) ---
# am243x_add_executable(test2_mismatch src/main.c)
# target_link_libraries(test2_mismatch PRIVATE linker_am243x_internal mylib_tms570)

# --- Test 3: Platform mismatch on libraries (COMPATIBLE_INTERFACE_STRING catches this) ---
# tms570_add_library(test3_mismatch src/mylib.c)
# target_link_libraries(test3_mismatch PRIVATE mylib_am243x)

# --- Test 4: Missing linker script (validator catches this) ---
# am243x_add_executable(test4_missing_linker_script src/main.c)
# target_link_libraries(test4_missing_linker_script PRIVATE mylib_am243x)

# --- Test 5: Naked library without wrapper (validator catches this) ---
# add_library(naked_lib STATIC src/mylib.c)

# --- Test 6: Naked executable without wrapper (validator catches this) ---
# add_executable(naked_exe STATIC src/main.c)

# --- Test 7: IMPORTED library without EMB_HAS_PLATFORM (validator catches this) ---
# add_library(test7_imported_no_platform STATIC IMPORTED)
# set_target_properties(test7_imported_no_platform PROPERTIES
#     IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libvendor_am243x.a"
# )

# --- Test 8: IMPORTED library with wrong platform (COMPATIBLE_INTERFACE_STRING catches this) ---
# add_library(test8_imported_wrong_platform STATIC IMPORTED)
# set_target_properties(test8_imported_wrong_platform PROPERTIES
#     IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/libvendor_am243x.a"
#     EMB_HAS_PLATFORM TRUE
#     INTERFACE_EMB_PLATFORM "TMS570"
# )
# set_property(TARGET test8_imported_wrong_platform APPEND PROPERTY
#     COMPATIBLE_INTERFACE_STRING EMB_PLATFORM)
# am243x_add_executable(test8_exe src/main.c)
# target_link_libraries(test8_exe PRIVATE test8_imported_wrong_platform linker_am243x_internal)

# --- Test 9: Two linker scripts ---
# am243x_add_executable(test9_two_linkers src/main.c)
# target_link_libraries(test9_two_linkers PRIVATE linker_am243x_internal linker_am243x_external)

# =============================================================================
# Validation - catches missing platform flags and linker scripts
# =============================================================================

emb_validate_all_targets_have_platform()
emb_validate_all_executables_have_linker_script()

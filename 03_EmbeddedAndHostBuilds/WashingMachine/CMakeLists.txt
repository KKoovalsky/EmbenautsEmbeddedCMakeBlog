cmake_minimum_required(VERSION 3.11)

include(ExternalProject)

project(WashingMachine)

# TODO: Download, install catch2 and use the downloaded version of Catch. Maybe there is a tar archive with Catch2
# out there?

set(JUNGLES_OS_HELPERS_DIR ${CMAKE_BINARY_DIR}/jungles_os_helpers_repo)
ExternalProject_Add(jungles_os_helpers_repo
    GIT_REPOSITORY https://github.com/KKoovalsky/JunglesOsHelpers.git
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    SOURCE_DIR ${JUNGLES_OS_HELPERS_DIR}
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

set(CPPTIME_DIR ${CMAKE_BINARY_DIR}/cpptime_repo)
ExternalProject_Add(cpptime_repo
    GIT_REPOSITORY https://github.com/eglimi/cpptime.git
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    SOURCE_DIR ${CPPTIME_DIR}
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

set(CXXOPTS_DIR ${CMAKE_BINARY_DIR}/cxxopts_repo)
ExternalProject_Add(cxxopts_repo
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    SOURCE_DIR ${CXXOPTS_DIR}
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
) 

set(CATCH2_DIR ${CMAKE_BINARY_DIR}/catch2_repo)
ExternalProject_Add(catch2_repo
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    SOURCE_DIR ${CATCH2_DIR}
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

set(TESTS_DIR ${CMAKE_SOURCE_DIR}/tests)
add_executable(WashingMachineTests
    ${TESTS_DIR}/main.cpp
    ${TESTS_DIR}/test_washing_cycle.cpp
    ${TESTS_DIR}/test_spinning_cycle.cpp
    ${TESTS_DIR}/test_drying_cycle.cpp
    ${TESTS_DIR}/test_cycles.cpp
    ${TESTS_DIR}/test_temperature_change_simulation.cpp
    ${TESTS_DIR}/test_water_flow_simulation.cpp
    ${TESTS_DIR}/test_job_delayer.cpp
    ${TESTS_DIR}/test_job_repeater.cpp
)
add_dependencies(WashingMachineTests jungles_os_helpers_repo cpptime_repo catch2_repo)
target_include_directories(WashingMachineTests PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${JUNGLES_OS_HELPERS_DIR}
    ${CPPTIME_DIR}
    ${CATCH2_DIR}/single_include
)
target_compile_features(WashingMachineTests PRIVATE cxx_std_17)
target_link_libraries(WashingMachineTests Threads::Threads)

add_custom_target(run_tests
    COMMAND valgrind --leak-check=full -q ./WashingMachineTests
)
add_dependencies(run_tests WashingMachineTests)

set(APP_DIR ${CMAKE_SOURCE_DIR}/app)
add_executable(WashingMachine
    ${APP_DIR}/main.cpp
)
add_dependencies(WashingMachine jungles_os_helpers_repo cpptime_repo cxxopts_repo)
target_include_directories(WashingMachine PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${JUNGLES_OS_HELPERS_DIR}
    ${CPPTIME_DIR}
    ${CXXOPTS_DIR}/include
)
target_compile_features(WashingMachine PRIVATE cxx_std_17)
target_link_libraries(WashingMachine Threads::Threads)

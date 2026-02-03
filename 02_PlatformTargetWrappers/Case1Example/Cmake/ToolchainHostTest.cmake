# ToolchainHostTest.cmake
# Test toolchain using host compiler (for verifying CMake logic)
# In real use, replace with Toolchain.cmake

# Use host compiler - just simulate the embedded setup
set(CMAKE_C_FLAGS_INIT "-DHOST_TEST")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")

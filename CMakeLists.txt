# xormove - Cross-platform file swap utility using XOR operations
# Swaps files between near-full disks without requiring temporary space

cmake_minimum_required(VERSION 3.20)

# CMake 3.30+ removed FindBoost module, use config-mode instead
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(xormove
    VERSION 0.1.0
    DESCRIPTION "Swap files between disks using XOR operations - no extra space needed"
    HOMEPAGE_URL "https://github.com/DazzleTools/xormove"
    LANGUAGES CXX
)

# C++17 for std::filesystem fallback and modern features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find dependencies via vcpkg (CONFIG mode for vcpkg packages)
find_package(Boost CONFIG REQUIRED COMPONENTS filesystem)
find_package(cryptopp CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

# Main executable (named xmv for short, project is xormove)
add_executable(xmv
    src/xormove.cpp
)

# Link libraries
target_link_libraries(xmv PRIVATE
    Boost::filesystem
    cryptopp::cryptopp
    argparse::argparse
)

# Include version header
target_include_directories(xmv PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Platform-specific settings
if(WIN32)
    # Windows-specific flags
    target_compile_definitions(xmv PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

if(MSVC)
    # MSVC-specific warning level
    target_compile_options(xmv PRIVATE /W4)
else()
    # GCC/Clang warning flags
    target_compile_options(xmv PRIVATE -Wall -Wextra -Wpedantic)
endif()

# =============================================================================
# Testing
# =============================================================================
enable_testing()

# Test executable for XOR swap algorithm
add_executable(test_xor_swap
    tests/test_xor_swap.cpp
)

if(MSVC)
    target_compile_options(test_xor_swap PRIVATE /W4)
else()
    target_compile_options(test_xor_swap PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Register test with CTest
add_test(NAME XorSwapTests COMMAND test_xor_swap)

# Test executable for path preservation features
add_executable(test_path_preservation
    tests/test_path_preservation.cpp
)

target_link_libraries(test_path_preservation PRIVATE
    Boost::filesystem
)

if(MSVC)
    target_compile_options(test_path_preservation PRIVATE /W4)
else()
    target_compile_options(test_path_preservation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME PathPreservationTests COMMAND test_path_preservation)

# =============================================================================
# Install
# =============================================================================
install(TARGETS xmv
    RUNTIME DESTINATION bin
)

# =============================================================================
# CPack for creating installers
# =============================================================================
include(CPack)
set(CPACK_PACKAGE_NAME "xormove")
set(CPACK_PACKAGE_VENDOR "DazzleTools")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "File swap utility for space-constrained disks")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

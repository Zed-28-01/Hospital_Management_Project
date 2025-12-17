cmake_minimum_required(VERSION 3.10)
project(HospitalManagement VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# C++ Standard Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type (Debug/Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/model
    ${PROJECT_SOURCE_DIR}/include/dal
    ${PROJECT_SOURCE_DIR}/include/bll
    ${PROJECT_SOURCE_DIR}/include/ui
    ${PROJECT_SOURCE_DIR}/include/common
    ${PROJECT_SOURCE_DIR}/include/advance
)

# ============================================================================
# Source Files - Organized by Layer
# ============================================================================

# Model Layer Sources
file(GLOB MODEL_SOURCES
    "src/model/*.cpp"
)

# Data Access Layer Sources
file(GLOB DAL_SOURCES
    "src/dal/*.cpp"
)

# Business Logic Layer Sources
file(GLOB BLL_SOURCES
    "src/bll/*.cpp"
)

# UI/Presentation Layer Sources
file(GLOB UI_SOURCES
    "src/ui/*.cpp"
)

# Common Utilities Sources
file(GLOB COMMON_SOURCES
    "src/common/*.cpp"
)

# Combine all sources (excluding main.cpp for library)
set(HMS_LIB_SOURCES
    ${MODEL_SOURCES}
    ${DAL_SOURCES}
    ${BLL_SOURCES}
    ${UI_SOURCES}
    ${COMMON_SOURCES}
)

# Main source
set(MAIN_SOURCE "src/main.cpp")

# ============================================================================
# Library Target (for code reuse between app and tests)
# ============================================================================
add_library(HospitalLib STATIC ${HMS_LIB_SOURCES})
target_include_directories(HospitalLib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ============================================================================
# Main Application Executable
# ============================================================================
add_executable(HospitalApp ${MAIN_SOURCE})
target_link_libraries(HospitalApp PRIVATE HospitalLib)

# Set working directory for data files
set_target_properties(HospitalApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============================================================================
# Testing with Google Test
# ============================================================================
enable_testing()
find_package(GTest REQUIRED)

# Test source files - organized by layer
file(GLOB MODEL_TEST_SOURCES "test/model/*.cpp")
file(GLOB DAL_TEST_SOURCES "test/dal/*.cpp")
file(GLOB BLL_TEST_SOURCES "test/bll/*.cpp")
file(GLOB INTEGRATION_TEST_SOURCES "test/integration/*.cpp")

# Combine all test sources
set(ALL_TEST_SOURCES
    ${MODEL_TEST_SOURCES}
    ${DAL_TEST_SOURCES}
    ${BLL_TEST_SOURCES}
    ${INTEGRATION_TEST_SOURCES}
)

# Check if test main exists, otherwise use GTest::Main
if(EXISTS "${PROJECT_SOURCE_DIR}/test/test_main.cpp")
    list(APPEND ALL_TEST_SOURCES "test/test_main.cpp")
    set(USE_GTEST_MAIN OFF)
else()
    set(USE_GTEST_MAIN ON)
endif()

# Create test executable if test sources exist
if(ALL_TEST_SOURCES)
    add_executable(HospitalTests ${ALL_TEST_SOURCES})
    target_link_libraries(HospitalTests
        PRIVATE
        HospitalLib
        GTest::GTest
        pthread
    )

    if(USE_GTEST_MAIN)
        target_link_libraries(HospitalTests PRIVATE GTest::Main)
    endif()

    # Add test to CTest
    add_test(NAME HospitalTests COMMAND HospitalTests)

    # Set working directory for test data
    set_target_properties(HospitalTests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Copy test fixtures to build directory
    add_custom_command(TARGET HospitalTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/test/fixtures
        ${CMAKE_BINARY_DIR}/test/fixtures
        COMMENT "Copying test fixtures..."
    )
endif()

# ============================================================================
# Copy Data Files to Build Directory
# ============================================================================
add_custom_command(TARGET HospitalApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/data
    ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data files to build directory..."
)

# ============================================================================
# Installation (Optional)
# ============================================================================
install(TARGETS HospitalApp DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/ DESTINATION share/hospitalmanagement/data)

# ============================================================================
# Print Configuration Summary
# ============================================================================
message(STATUS "")
message(STATUS "=== Hospital Management System Build Configuration ===")
message(STATUS "CMake version:     ${CMAKE_VERSION}")
message(STATUS "C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Project version:   ${PROJECT_VERSION}")
message(STATUS "")
message(STATUS "Source directories:")
message(STATUS "  Model:           src/model/")
message(STATUS "  DAL:             src/dal/")
message(STATUS "  BLL:             src/bll/")
message(STATUS "  UI:              src/ui/")
message(STATUS "  Common:          src/common/")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  HospitalApp      - Main application")
message(STATUS "  HospitalLib      - Static library")
message(STATUS "  HospitalTests    - Unit tests (GTest)")
message(STATUS "======================================================")
message(STATUS "")

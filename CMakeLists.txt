cmake_minimum_required(VERSION 3.10)
project(FallDetectionTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Google Test
include(FetchContent)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

# Include directories (test directory first so we find our mock Arduino.h)
include_directories(
    ${PROJECT_SOURCE_DIR}/test
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# Create test executable
add_executable(
    fall_detection_tests
    test/test_state_machine.cpp
    src/logic/FallDetector.cpp
    test/SerialImpl.cpp
    test/helpers/TimeSimulator.cpp
)

# Add compile definitions for testing
target_compile_definitions(fall_detection_tests PRIVATE TESTING_MODE)

# Link with Google Test
target_link_libraries(
    fall_detection_tests
    gtest_main
    gtest
)

# Add test discovery
enable_testing()
add_test(
    NAME FallDetectorTests
    COMMAND fall_detection_tests
)

# Add the project root to the include path so "test/helpers/..." works
target_include_directories(fall_detection_tests PRIVATE "${CMAKE_SOURCE_DIR}")

# Set working directory for tests
set_tests_properties(
    FallDetectorTests
    PROPERTIES
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

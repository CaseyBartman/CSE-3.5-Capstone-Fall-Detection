#!/bin/bash

##############################################################################
#
# Fall Detection Test Suite Runner
#
# Usage: ./run_tests.sh [OPTION]
# Options:
#   -c, --clean      Clean previous build artifacts
#   -v, --verbose    Show verbose build output
#   -h, --help       Display this help message
#
# Example:
#   ./run_tests.sh
#   ./run_tests.sh --clean
#   ./run_tests.sh --verbose
#
##############################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Detect platform and set TEST_BINARY path accordingly
# Windows MSBuild puts binaries in Debug/Release subdirectories
if [ -f "$BUILD_DIR/Debug/fall_detection_tests.exe" ]; then
    TEST_BINARY="$BUILD_DIR/Debug/fall_detection_tests.exe"
elif [ -f "$BUILD_DIR/Release/fall_detection_tests.exe" ]; then
    TEST_BINARY="$BUILD_DIR/Release/fall_detection_tests.exe"
else
    # Fallback to Unix-style path (for non-Windows or Makefile-based builds)
    TEST_BINARY="$BUILD_DIR/fall_detection_tests"
fi

CLEAN=false
VERBOSE=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            head -20 "$0" | tail -19
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Print header
echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}   Fall Detection System - Test Suite${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
echo ""

# Check for required tools
echo -e "${YELLOW}Checking prerequisites...${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: CMake not found. Please install CMake.${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ CMake found: $(cmake --version | head -1)${NC}"

if ! command -v make &> /dev/null; then
    echo -e "${RED}ERROR: Make not found. Please install make.${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ Make found${NC}"

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo ""
    echo -e "${YELLOW}Cleaning previous builds...${NC}"
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        echo -e "${GREEN}  ✓ Build directory cleaned${NC}"
    fi
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
    echo -e "${GREEN}  ✓ Build directory created${NC}"
fi

# Configure CMake
echo ""
echo -e "${YELLOW}Configuring CMake project...${NC}"
cd "$BUILD_DIR"

if [ "$VERBOSE" = true ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Debug
else
    cmake .. -DCMAKE_BUILD_TYPE=Debug > /dev/null 2>&1
fi

if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: CMake configuration failed${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ CMake configuration complete${NC}"

# Build tests
echo -e "${YELLOW}Building test suite...${NC}"
if [ "$VERBOSE" = true ]; then
    cmake --build .
else
    cmake --build . > /dev/null 2>&1
fi

if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: Build failed${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ Build successful${NC}"

# Run tests
echo ""
echo -e "${YELLOW}Running tests...${NC}"
echo -e "${CYAN}─────────────────────────────────────────────────────────────${NC}"
echo ""

cd "$SCRIPT_DIR"
"$TEST_BINARY"
TEST_RESULT=$?

echo ""
echo -e "${CYAN}─────────────────────────────────────────────────────────────${NC}"

# Display results
echo ""
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}   Test Suite Complete - Success${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
else
    echo -e "${RED}✗ Tests failed with exit code: $TEST_RESULT${NC}"
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${RED}   Test Suite Complete - Failures Detected${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
fi

exit $TEST_RESULT

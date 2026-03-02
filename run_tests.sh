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
TEST_BINARY=""

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

# Detect test binary path after successful build
# Windows MSBuild puts binaries in Debug/Release subdirectories
if [ -f "$BUILD_DIR/Debug/fall_detection_tests.exe" ]; then
    TEST_BINARY="$BUILD_DIR/Debug/fall_detection_tests.exe"
elif [ -f "$BUILD_DIR/Release/fall_detection_tests.exe" ]; then
    TEST_BINARY="$BUILD_DIR/Release/fall_detection_tests.exe"
elif [ -f "$BUILD_DIR/fall_detection_tests" ]; then
    # Unix-style path (for non-Windows or Makefile-based builds)
    TEST_BINARY="$BUILD_DIR/fall_detection_tests"
else
    echo -e "${RED}ERROR: Test binary not found after successful build${NC}"
    exit 1
fi

# Run tests
echo ""
echo -e "${YELLOW}Running tests...${NC}"
echo ""

cd "$SCRIPT_DIR"
# Check if Python is available for better output formatting
# Note: Check 'python' before 'python3' because python3 may be a Windows Store alias
PYTHON_CMD=""
if command -v python &> /dev/null; then
    PYTHON_CMD="python"
elif command -v python3 &> /dev/null; then
    PYTHON_CMD="python3"
fi

if [ -n "$PYTHON_CMD" ] && [ -f "$SCRIPT_DIR/parse_tests.py" ]; then
    # Use Python parser for clean formatted output
    $PYTHON_CMD "$SCRIPT_DIR/parse_tests.py" "$TEST_BINARY"
    TEST_RESULT=$?
else
    # Fallback to direct execution
    if [ -z "$PYTHON_CMD" ]; then
        echo -e "${YELLOW}Note: Python not found, using default output format${NC}"
    fi
    echo -e "${CYAN}─────────────────────────────────────────────────────────────${NC}"
    echo ""
    "$TEST_BINARY"
    TEST_RESULT=$?
    echo ""
    echo -e "${CYAN}─────────────────────────────────────────────────────────────${NC}"
fi

exit $TEST_RESULT

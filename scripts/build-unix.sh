#!/bin/bash
# xormove Unix/Linux/macOS Build Script
# Works on Linux, macOS, BSD, and WSL

set -e

# Navigate to project root
cd "$(dirname "$0")/.."
PROJECT_ROOT=$(pwd)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo_info() { echo -e "${GREEN}[INFO]${NC} $1"; }
echo_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
echo_error() { echo -e "${RED}[ERROR]${NC} $1"; }

show_help() {
    echo "xormove Unix Build Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --help, -h      Show this help message"
    echo "  --debug         Build debug configuration"
    echo "  --release       Build release configuration (default)"
    echo "  --clean         Clean build directory before building"
    echo "  --rebuild       Same as --clean"
    echo "  --test          Run tests after building"
    echo "  --configure     Configure only, don't build"
    echo "  --preset        Use CMake presets instead of manual configuration"
    echo "  --verbose       Verbose build output"
    echo ""
    echo "Examples:"
    echo "  $0                        # Release build"
    echo "  $0 --debug                # Debug build"
    echo "  $0 --clean --test         # Clean rebuild with tests"
    echo "  $0 --preset               # Use CMake presets"
    echo ""
}

# Check for required tools
check_tools() {
    if ! command -v cmake &> /dev/null; then
        echo_error "CMake not found. Install with:"
        echo "  Ubuntu/Debian: sudo apt install cmake"
        echo "  macOS: brew install cmake"
        echo "  Fedora: sudo dnf install cmake"
        exit 1
    fi

    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        echo_error "No C++ compiler found. Install with:"
        echo "  Ubuntu/Debian: sudo apt install build-essential"
        echo "  macOS: xcode-select --install"
        exit 1
    fi
}

# Find vcpkg
find_vcpkg() {
    if [ -z "$VCPKG_ROOT" ]; then
        # Check common locations
        if [ -d "$HOME/vcpkg" ]; then
            export VCPKG_ROOT="$HOME/vcpkg"
        elif [ -d "/opt/vcpkg" ]; then
            export VCPKG_ROOT="/opt/vcpkg"
        elif [ -d "/usr/local/vcpkg" ]; then
            export VCPKG_ROOT="/usr/local/vcpkg"
        else
            echo_warn "vcpkg not found. Will attempt to bootstrap..."

            # Offer to install vcpkg
            read -p "Install vcpkg to ~/vcpkg? [Y/n] " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
                git clone https://github.com/Microsoft/vcpkg.git "$HOME/vcpkg"
                "$HOME/vcpkg/bootstrap-vcpkg.sh"
                export VCPKG_ROOT="$HOME/vcpkg"
            else
                echo_error "vcpkg required. Set VCPKG_ROOT or install vcpkg."
                exit 1
            fi
        fi
    fi

    echo_info "Using vcpkg at: $VCPKG_ROOT"
}

# Detect OS for preset name
get_preset_os() {
    case "$(uname -s)" in
        Linux*)  echo "linux";;
        Darwin*) echo "macos";;
        *)       echo "linux";;  # Default to linux for BSD etc.
    esac
}

# Parse arguments
BUILD_TYPE="Release"
CLEAN_BUILD=0
VERBOSE=0
RUN_TESTS=0
USE_PRESET=0
CONFIGURE_ONLY=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean|--rebuild)
            CLEAN_BUILD=1
            shift
            ;;
        --test)
            RUN_TESTS=1
            shift
            ;;
        --preset)
            USE_PRESET=1
            shift
            ;;
        --configure)
            CONFIGURE_ONLY=1
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        *)
            echo_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Check tools first
check_tools

# Use CMake presets if requested
if [ $USE_PRESET -eq 1 ]; then
    echo_info "Using CMake presets..."
    OS_NAME=$(get_preset_os)
    PRESET_TYPE=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
    PRESET_NAME="${OS_NAME}-${PRESET_TYPE}"

    if [ $CLEAN_BUILD -eq 1 ]; then
        echo_info "Cleaning build directory..."
        rm -rf "build/${OS_NAME}-release" "build/${OS_NAME}-debug"
    fi

    echo_info "Configuring with preset: $PRESET_NAME"
    cmake --preset "$PRESET_NAME"

    if [ $CONFIGURE_ONLY -eq 1 ]; then
        echo_info "Configuration complete."
        exit 0
    fi

    echo_info "Building with preset: $PRESET_NAME"
    cmake --build --preset "$PRESET_NAME"

    if [ $RUN_TESTS -eq 1 ]; then
        echo_info "Running tests..."
        ctest --preset "$PRESET_NAME"
    fi

    echo_info "Build successful!"
    exit 0
fi

# Find vcpkg for non-preset builds
find_vcpkg

# Clean build directory if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo_info "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
mkdir -p build

# Configure with CMake
echo_info "Configuring CMake ($BUILD_TYPE)..."
cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

if [ $CONFIGURE_ONLY -eq 1 ]; then
    echo_info "Configuration complete."
    exit 0
fi

# Build
echo_info "Building..."
if [ $VERBOSE -eq 1 ]; then
    cmake --build build --config "$BUILD_TYPE" --verbose
else
    cmake --build build --config "$BUILD_TYPE"
fi

# Run tests if requested
if [ $RUN_TESTS -eq 1 ]; then
    echo_info "Running tests..."
    ctest --test-dir build --build-config "$BUILD_TYPE" --output-on-failure
fi

echo_info "Build successful!"
echo_info "Executable: build/xormove"

# Show file info
if [ -f "build/xormove" ]; then
    ls -lh build/xormove
fi

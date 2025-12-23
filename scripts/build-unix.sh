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

# Check for required tools
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

# Find vcpkg
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

# Parse arguments
BUILD_TYPE="Release"
CLEAN_BUILD=0
VERBOSE=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        *)
            echo_error "Unknown option: $1"
            echo "Usage: $0 [--debug|--release] [--clean] [--verbose]"
            exit 1
            ;;
    esac
done

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

# Build
echo_info "Building..."
if [ $VERBOSE -eq 1 ]; then
    cmake --build build --config "$BUILD_TYPE" --verbose
else
    cmake --build build --config "$BUILD_TYPE"
fi

echo_info "Build successful!"
echo_info "Executable: build/xormove"

# Show file info
if [ -f "build/xormove" ]; then
    ls -lh build/xormove
fi

# Building xormove

This guide covers building xormove on Windows, Linux, and macOS using various development environments.

## Prerequisites

All platforms require:
- **CMake 3.20+** - Build system generator
- **vcpkg** - C++ package manager (handles all dependencies)
- **C++17 compiler** - MSVC, GCC 8+, or Clang 7+

## Windows

### Option 1: Visual Studio 2022 (Recommended)

[Download Visual Studio 2022 Community](https://visualstudio.microsoft.com/vs/community/) (free)

During installation, select:
- **Desktop development with C++** workload
- **C++ CMake tools for Windows** (includes vcpkg)

**Build steps:**

1. Open Visual Studio
2. Select **File > Open > Folder** and choose the xormove directory
3. Visual Studio auto-detects CMakeLists.txt and configures the project
4. Select build configuration (Debug/Release) from the toolbar
5. Build with **Build > Build All** or press `Ctrl+Shift+B`

The executable will be at `build/Release/xmv.exe` or `build/Debug/xmv.exe`.

### Option 2: Visual Studio Code

[Download VS Code](https://code.visualstudio.com/) (free)

**Required extensions:**
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) - IntelliSense and debugging
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) - CMake integration

**Setup vcpkg** (if not using Visual Studio's bundled vcpkg):

```powershell
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg

# Bootstrap
.\bootstrap-vcpkg.bat

# Set environment variable (add to system PATH for persistence)
$env:VCPKG_ROOT = "C:\vcpkg"
```

**Build steps:**

1. Open the xormove folder in VS Code
2. When prompted, select a kit (e.g., "Visual Studio Community 2022 Release - amd64")
3. CMake Tools will configure automatically
4. Click **Build** in the status bar or press `F7`

**Debugging:**

Press `F5` to start debugging. The included `.vscode/launch.json` is pre-configured for the test files.

### Option 3: Command Line (Developer PowerShell)

Open **Developer PowerShell for VS 2022** from the Start menu:

```powershell
cd path\to\xormove

# Configure (uses vcpkg from Visual Studio)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"

# Build Release
cmake --build build --config Release

# Build Debug
cmake --build build --config Debug
```

Or use the provided script:

```cmd
scripts\build-windows.cmd
```

## Linux

### Ubuntu/Debian

```bash
# Install build tools
sudo apt update
sudo apt install -y build-essential cmake git curl zip unzip tar pkg-config

# Clone and bootstrap vcpkg
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg
export PATH=$VCPKG_ROOT:$PATH

# Add to ~/.bashrc for persistence
echo 'export VCPKG_ROOT=~/vcpkg' >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
```

### Fedora/RHEL

```bash
sudo dnf install -y gcc-c++ cmake git curl zip unzip tar pkgconfig
# Then follow vcpkg steps above
```

### Build

```bash
cd xormove

# Configure
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build build --config Release

# Run tests
cd build && ctest --output-on-failure
```

Or use the provided script:

```bash
chmod +x scripts/build-unix.sh
./scripts/build-unix.sh
```

## macOS

### Prerequisites

```bash
# Install Xcode command line tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake
brew install cmake

# Clone and bootstrap vcpkg
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg

# Add to ~/.zshrc for persistence
echo 'export VCPKG_ROOT=~/vcpkg' >> ~/.zshrc
```

### Build

```bash
cd xormove

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

Or use the provided script:

```bash
./scripts/build-unix.sh
```

## Build Options

| Option | Description |
|--------|-------------|
| `-DCMAKE_BUILD_TYPE=Release` | Optimized build (default) |
| `-DCMAKE_BUILD_TYPE=Debug` | Debug symbols, no optimization |
| `-DVCPKG_TARGET_TRIPLET=x64-windows-static` | Static linking (Windows) |

## Troubleshooting

### vcpkg not found

Ensure `VCPKG_ROOT` environment variable is set and vcpkg is bootstrapped:

```bash
echo $VCPKG_ROOT  # Should show path to vcpkg
ls $VCPKG_ROOT/vcpkg  # Should show the vcpkg executable
```

### CMake can't find compiler

**Windows:** Use Developer PowerShell or ensure Visual Studio C++ tools are installed.

**Linux:** Install `build-essential` (Ubuntu) or `gcc-c++` (Fedora).

**macOS:** Run `xcode-select --install`.

### Dependency build failures

Try updating vcpkg:

```bash
cd $VCPKG_ROOT
git pull
./bootstrap-vcpkg.sh  # or .bat on Windows
```

### Clean rebuild

```bash
# Remove build directory and vcpkg packages
rm -rf build/
rm -rf vcpkg_installed/

# Reconfigure and build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

## IDE-Specific Notes

### CLion

1. Open the project folder
2. Go to **Settings > Build, Execution, Deployment > CMake**
3. Add to CMake options: `-DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`
4. Reload CMake project

### Qt Creator

1. Open CMakeLists.txt as project
2. In Projects mode, add to CMake configuration: `CMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`
3. Run CMake and build

## Running Tests

After building:

```bash
cd build
ctest -C Release --output-on-failure
```

Or run the test executables directly:

```bash
# XOR swap algorithm tests
./build/Release/test_xor_swap        # Windows: .\build\Release\test_xor_swap.exe

# Path preservation tests
./build/Release/test_path_preservation  # Windows: .\build\Release\test_path_preservation.exe
```

### Test Coverage

| Test Suite | Description |
|------------|-------------|
| `test_xor_swap` | Core XOR swap algorithm verification |
| `test_path_preservation` | Path keyword parsing and destination resolution |

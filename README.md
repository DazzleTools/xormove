# xormove

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20BSD-lightgrey)]()

**Swap files between near-full disks without requiring temporary space.**

## The Problem

You have two disks, both nearly full. You need to swap files between them:

```
Disk1: FileA (10GB)     Disk2: FileB (8GB)
       Free: 2GB               Free: 3GB
```

**Traditional copy fails** - neither disk has enough free space to hold the other's file as a temporary copy.

## The Solution

xormove uses the XOR swap algorithm to exchange file contents in-place, chunk by chunk:

```
A XOR B XOR A = B
A XOR B XOR B = A
```

By streaming chunks and XORing them together, xormove swaps the contents **without needing extra disk space** beyond a small buffer.

## Features

- **Space-efficient**: Only needs buffer space, not full file size
- **Integrity verification**: Optional SHA-256 hash verification
- **Progress display**: Track swap progress for large files
- **Cross-platform**: Windows, Linux, macOS, BSD
- **Safe by default**: Uses temp files with atomic rename

## Installation

### Prerequisites

- CMake 3.20+
- vcpkg (bundled with Visual Studio 2022, or install separately)
- C++17 compiler (MSVC, GCC, Clang)

### Build from Source

**Windows (Visual Studio):**
```cmd
scripts\build-windows.cmd
```

**Linux/macOS/BSD:**
```bash
chmod +x scripts/build-unix.sh
./scripts/build-unix.sh
```

**Manual CMake:**
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

## Usage

```bash
# Basic file swap
xormove /path/to/fileA /path/to/fileB

# With integrity verification
xormove fileA fileB --verify

# With progress display
xormove fileA fileB --progress

# Verbose output with logging
xormove fileA fileB --verbose --log swap.log

# Secure mode (larger chunks, more thorough)
xormove fileA fileB --secure --verify
```

### Options

| Option | Description |
|--------|-------------|
| `--secure` | Use larger chunk size (1MB vs 4KB) |
| `--fast` | Minimal checking for speed |
| `--verify` | SHA-256 verification after swap |
| `--verbose`, `-vb` | Detailed output |
| `--log FILE` | Write to log file |
| `--progress` | Display progress bar |

## How It Works

1. **Pre-flight checks**: Verify both files exist, check disk space
2. **Chunk streaming**: Read matching chunks from both files
3. **XOR transformation**: Apply XOR to swap chunk contents
4. **Safe write**: Write to temporary files first
5. **Atomic swap**: Rename temp files to final destinations
6. **Verification** (optional): Hash check to confirm integrity

## Use Cases

- **Embedded systems**: Limited storage devices
- **Thumb drives**: Reorganizing files on small USB drives
- **Archive management**: Swapping large backup files
- **Disk cleanup**: Moving files when space is tight

## Dependencies

Managed via [vcpkg](https://vcpkg.io/):

- Boost.Filesystem
- Boost.Algorithm
- Crypto++ (for SHA-256)
- argparse

## Development

### Project Structure

```
xormove/
├── CMakeLists.txt      # Build configuration
├── vcpkg.json          # Dependency manifest
├── src/
│   └── xormove.cpp     # Main source
├── scripts/
│   ├── build-windows.cmd
│   └── build-unix.sh
└── tests/              # Test files
```

### Building for Development

```bash
# Debug build
./scripts/build-unix.sh --debug

# Clean rebuild
./scripts/build-unix.sh --clean --debug
```

## Contributions

Contributions are welcome! Please read our [Contributing Guide](CONTRIBUTING.md) for details.

Like the project?

[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/djdarcy)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

Created by Dustin ([@djdarcy](https://github.com/djdarcy))

Part of the [DazzleTools](https://github.com/DazzleTools) collection.

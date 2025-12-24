# Contributing to xormove

## Development Setup

### Prerequisites

- Visual Studio 2022 with C++17 support
- Pre-built Boost 1.85.0 libraries (filesystem, timer, algorithm)
- Pre-built Crypto++ 8.9.0 libraries
- argparse 3.0 header-only library

### Library Setup

The project expects pre-built libraries in a sibling directory structure:

```
C:\code\
├── xormove\
│   └── local\          # This repository
│       ├── src\
│       ├── include\
│       └── xormove.sln
└── xormove_orig\
    └── libs\           # Pre-built dependencies
        ├── boost_1_85_0\
        │   └── stage\lib\  # Compiled Boost libs
        ├── cryptopp890\
        │   └── cryptopp\x64\  # Compiled Crypto++ libs
        └── argparse-3.0\
            └── include\    # Header-only
```

### Building

1. Open `xormove.sln` in Visual Studio 2022
2. Select configuration (Debug|x64 recommended for development)
3. Build Solution (Ctrl+Shift+B)

### Testing

Create test files in `test-runs/` directory:

```bash
echo "Test content for file A" > test-runs/fileA.txt
echo "Test content for file B" > test-runs/fileB.txt
```

Run the executable:

```bash
bin\Debug\xmv.exe test-runs\fileA.txt test-runs\fileB.txt --verbose --progress
```

## Code Style

- C++17 standard
- Use `boost::filesystem` for file operations
- Prefer descriptive variable names
- Add comments for non-obvious logic

## Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request with clear description

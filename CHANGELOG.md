# Changelog

All notable changes to xormove will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2025-12-24

### Added
- **Path preservation options** (`--1-to`, `--2-to`) for controlling file destinations
- Path keywords: `REL`, `SAME-AS-1`, `SAME-AS-2` for flexible destination control
- Interactive prompts for directory creation and file overwrite confirmation
- `--yes` flag with action parameters (`mkdir`, `overwrite`, `all`) for automation
- Same-drive rename optimization for faster swaps on the same filesystem
- Enhanced `--dry-run` output showing path transformations and required actions

### Changed
- Default behavior when path flags used: unspecified files default to `REL`

## [0.2.0] - 2025-12-23

### Changed
- **Binary renamed from `xormove` to `xmv`** for easier command-line use
- `--version` now shows actual version instead of argparse default

### Added
- `--dry-run` flag to preview operations without making changes
- `ROADMAP.md` with development milestones and project board link

### Fixed
- Suppressed unused parameter warning for `fast` mode (planned for v0.5.0)

## [0.1.4] - 2025-12-23

### Fixed
- macOS CI build now uses arm64-osx triplet (GitHub macos-latest is now Apple Silicon)

## [0.1.3] - 2025-12-23

### Fixed
- GitHub Actions CI now works on Windows and macOS (use bash shell for cross-platform compatibility)
- CMake 3.30+ compatibility with CMP0167 policy for Boost module removal
- Use CONFIG mode for find_package(Boost) with vcpkg

### Changed
- Platform badges now use green color to distinguish from build tool badges

## [0.1.2] - 2025-12-23

### Fixed
- XOR swap now works correctly on files smaller than the chunk size (4096 bytes)
- File rename logic correctly swaps content instead of putting it back in original files
- Different-sized files now preserve their original sizes after swap

### Added
- Build documentation (`docs/BUILDING.md`) with instructions for Visual Studio, VS Code, CLion, and command-line builds
- README badges for build status, platforms, and project info
- Links to Visual Studio and vcpkg download pages in README

### Changed
- Updated vcpkg baseline to 2024.11.16 for CI compatibility
- Fixed VS Code launch.json debug path

## [0.1.1] - 2025-12-22

### Changed
- Replaced deprecated `boost::timer::progress_display` with custom `ProgressBar` class
- Enhanced build scripts for Windows and Unix platforms
- Added CMake presets for easier configuration

### Fixed
- Build compatibility with recent Boost versions

## [0.1.0] - 2025-12-22

### Added
- Initial release
- XOR swap algorithm for exchanging file contents without temporary space
- Cross-platform support (Windows, Linux, macOS, BSD)
- SHA-256 integrity verification (`--verify` flag)
- Progress bar display (`--progress` flag)
- Verbose output and logging options
- Secure mode with larger chunk size (`--secure` flag)
- CMake build system with vcpkg dependency management

[0.3.0]: https://github.com/DazzleTools/xormove/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/DazzleTools/xormove/compare/v0.1.4...v0.2.0
[0.1.4]: https://github.com/DazzleTools/xormove/compare/v0.1.3...v0.1.4
[0.1.3]: https://github.com/DazzleTools/xormove/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/DazzleTools/xormove/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/DazzleTools/xormove/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/DazzleTools/xormove/releases/tag/v0.1.0

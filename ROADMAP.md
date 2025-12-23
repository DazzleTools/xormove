# xormove Roadmap

> **Move files even when your disks are full.**
>
> Binary: `xmv` (short for "xor-move")

This document outlines the development direction for xormove. Each major feature links to a GitHub issue for detailed discussion and tracking.

**Project Board**: [xormove Development Roadmap](https://github.com/orgs/DazzleTools/projects/2)

## Current Release: v0.1.x (Core XOR Swap)

The 0.1.x series established the core functionality:

- **v0.1.0**: Initial release with XOR swap algorithm
- **v0.1.1**: Replaced deprecated Boost timer, enhanced build scripts
- **v0.1.2**: Fixed XOR swap bugs (small files, rename logic, size preservation)
- **v0.1.3**: Windows/macOS CI fixes, CMake 3.30+ compatibility
- **v0.1.4**: macOS ARM64 support

## Next: v0.2.0 (Essential Polish)

**Theme**: Quality-of-life improvements for daily use.

| Feature | Issue | Description |
|---------|-------|-------------|
| Binary Rename | [#5](https://github.com/DazzleTools/xormove/issues/5) | Binary now named `xmv` for easier typing |
| Version Fix | [#7](https://github.com/DazzleTools/xormove/issues/7) | Show actual version instead of "1.0" |
| Dry Run Mode | [#8](https://github.com/DazzleTools/xormove/issues/8) | Preview operations without executing |

## Planned: v0.3.0 (Move Operations)

**Theme**: Basic mv/move command compatibility.

| Feature | Issue | Description |
|---------|-------|-------------|
| Single-file Move | [#4](https://github.com/DazzleTools/xormove/issues/4) | `xmv file.txt /dest/` syntax |
| Overwrite Flags | [#4](https://github.com/DazzleTools/xormove/issues/4) | `-f`, `-i`, `-n` for overwrite control |
| Same-disk Rename | [#4](https://github.com/DazzleTools/xormove/issues/4) | Use fast rename for same-filesystem moves |

## Planned: v0.4.0 (Smart Mode)

**Theme**: Automatic strategy selection.

| Feature | Issue | Description |
|---------|-------|-------------|
| Path Preservation | [#1](https://github.com/DazzleTools/xormove/issues/1) | `--mirror-path`, `--dest-dir` options |
| Space Detection | [#4](https://github.com/DazzleTools/xormove/issues/4) | Auto-choose rename vs copy-delete vs XOR swap |
| Cross-disk Intelligence | [#4](https://github.com/DazzleTools/xormove/issues/4) | Detect filesystem boundaries |

## Planned: v0.5.0 (Optimization & Integration)

**Theme**: Performance improvements and ecosystem integration.

| Feature | Issue | Description |
|---------|-------|-------------|
| Fast Mode Optimization | [#3](https://github.com/DazzleTools/xormove/issues/3) | Skip XOR for identical byte sections |
| Preserve Integration | [#2](https://github.com/DazzleTools/xormove/issues/2) | Track file movements with preserve manifests |
| Recursive Directories | - | Move entire directory trees |

**Ecosystem Note**: xormove and [preserve](https://github.com/djdarcy/preserve) are complementary tools:
- xormove handles space-efficient file movement
- preserve tracks where files went for later restoration
- Together: Move files safely, always know where they came from

## Future: v1.0.0 (Production Ready)

**Theme**: Full mv/move replacement, complete documentation.

| Feature | Issue | Description |
|---------|-------|-------------|
| Full Compatibility | [#4](https://github.com/DazzleTools/xormove/issues/4) | Complete mv/move command compatibility |
| Glob Patterns | [#4](https://github.com/DazzleTools/xormove/issues/4) | Native `*.txt` pattern support |
| Man Page | - | Comprehensive documentation |

## Future: v2.0.0 (Rust Rewrite)

**Theme**: Single static binary, cross-compilation.

| Feature | Issue | Description |
|---------|-------|-------------|
| Rust Port | [#6](https://github.com/DazzleTools/xormove/issues/6) | Rewrite in Rust for static binary |
| Cross-compile | [#6](https://github.com/DazzleTools/xormove/issues/6) | Build all platforms from one machine |

**Prerequisite**: Complete C++ feature set (v1.0.0) before starting Rust port.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for how to get involved. Feature requests and bug reports are welcome via [GitHub Issues](https://github.com/DazzleTools/xormove/issues).

## Design Documents

Analysis documents are maintained in `private/claude/` for major features:
- `2025-12-23__15-56-13__xormove-roadmap-planning.md` - Roadmap planning analysis
- `2025-12-23__15-48-05__full-postmortem_xormove-v0.1.4-release.md` - v0.1.4 release postmortem

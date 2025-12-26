# xormove Roadmap

> **Move files even when your disks are full.**
>
> Binary: `xmv` (short for "xor-move")

This document outlines the development direction for xormove. Each major feature links to a GitHub issue for detailed discussion and tracking.

**Project Board**: [xormove Development Roadmap](https://github.com/orgs/DazzleTools/projects/2)

## Release History

### v0.1.x (Core XOR Swap)

The 0.1.x series established the core functionality:

- **v0.1.0**: Initial release with XOR swap algorithm
- **v0.1.1**: Replaced deprecated Boost timer, enhanced build scripts
- **v0.1.2**: Fixed XOR swap bugs (small files, rename logic, size preservation)
- **v0.1.3**: Windows/macOS CI fixes, CMake 3.30+ compatibility
- **v0.1.4**: macOS ARM64 support

### v0.2.0 (Essential Polish)

**Theme**: Quality-of-life improvements for daily use.

| Feature | Issue | Description |
|---------|-------|-------------|
| Binary Rename | [#5](https://github.com/DazzleTools/xormove/issues/5) | Binary now named `xmv` for easier typing |
| Version Fix | [#7](https://github.com/DazzleTools/xormove/issues/7) | Show actual version instead of "1.0" |
| Dry Run Mode | [#8](https://github.com/DazzleTools/xormove/issues/8) | Preview operations without executing |

### v0.3.x (Path Preservation) ← Current Release

**Theme**: Flexible destination control for swapped files.

| Feature | Issue | Description |
|---------|-------|-------------|
| Path Options | [#1](https://github.com/DazzleTools/xormove/issues/1) | `--1-to`, `--2-to` flags for destination control |
| Path Keywords | [#1](https://github.com/DazzleTools/xormove/issues/1) | `REL`, `SAME`, `SAME-AS-1`, `SAME-AS-2` strategies |
| Auto-confirmation | - | `--yes` with `mkdir`, `overwrite`, `all` parameters |
| Interactive Prompts | - | Confirm directory creation and overwrites |
| Same-drive Optimization | [#15](https://github.com/DazzleTools/xormove/issues/15) | Atomic renames for same-drive operations |
| Smart Defaults | - | Context-aware defaults based on drive/folder |

**Releases**:
- **v0.3.0**: Path preservation implementation
- **v0.3.1**: Default behavior refinement (REL for cross-drive), unit tests
- **v0.3.2**: Same-drive swap = folder/name swap via atomic renames (planned)

**Default Behavior (v0.3.2)**:

| Scenario | Default Strategy | Implementation |
|----------|------------------|----------------|
| Cross-drive | REL/REL | XOR swap (space-efficient) |
| Same-drive, different folder | Folder swap | Atomic renames |
| Same-drive, same folder | Name swap | Atomic renames (three-way) |

## Current: v0.4.0 (Edge Cases & Polish)

**Theme**: Handle edge cases and improve robustness.

| Feature | Issue | Description |
|---------|-------|-------------|
| Symlink Detection | [#9](https://github.com/DazzleTools/xormove/issues/9) | Detect and handle junctions, symlinks, hardlinks |
| Long Paths | [#11](https://github.com/DazzleTools/xormove/issues/11) | Handle long paths and path flattening options |
| Keyword Validation | [#12](https://github.com/DazzleTools/xormove/issues/12) | Ensure keywords aren't parsed as literal paths |

## Planned: v0.5.0 (Move Operations & Profiles)

**Theme**: Basic mv/move command compatibility with profile-based behavior selection.

| Feature | Issue | Description |
|---------|-------|-------------|
| Single-file Move | [#4](https://github.com/DazzleTools/xormove/issues/4) | `xmv file.txt /dest/` syntax |
| Overwrite Flags | [#4](https://github.com/DazzleTools/xormove/issues/4) | `-f`, `-i`, `-n` for overwrite control |
| Profile System | [#16](https://github.com/DazzleTools/xormove/issues/16) | `--profile {swap\|mv\|mv-safe}` presets |
| Auto-detection | [#17](https://github.com/DazzleTools/xormove/issues/17) | Heuristic-based profile selection |
| Environment Vars | [#18](https://github.com/DazzleTools/xormove/issues/18) | `XMV_YES`, `XMV_PROFILE` defaults |
| UNC Paths | [#10](https://github.com/DazzleTools/xormove/issues/10) | Support network share paths |

**Profile System**: Profiles set sensible defaults for multiple dimensions:
- `--profile swap` (default): Symmetric swap, both files move, prompt on collision
- `--profile mv`: Traditional mv semantics, file1 → file2 location, error on collision

Profiles include collision defaults, but `--on-exist` and `--yes` flags always override.

## Planned: v0.6.0 (Optimization & Integration)

**Theme**: Performance improvements and ecosystem integration.

| Feature | Issue | Description |
|---------|-------|-------------|
| Fast Mode Optimization | [#3](https://github.com/DazzleTools/xormove/issues/3) | Skip XOR for identical byte sections |
| Preserve Integration | [#2](https://github.com/DazzleTools/xormove/issues/2) | Track file movements with preserve manifests |
| XOR Encryption | [#13](https://github.com/DazzleTools/xormove/issues/13) | Half-XOR with key file for encryption |
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
| Man Page | - | Documentation |

## Future: v2.0.0 (Rust Rewrite)

**Theme**: Single static binary, cross-compilation.

| Feature | Issue | Description |
|---------|-------|-------------|
| Rust Port | [#6](https://github.com/DazzleTools/xormove/issues/6) | Rewrite in Rust for static binary |
| Cross-compile | [#6](https://github.com/DazzleTools/xormove/issues/6) | Build all platforms from one machine |

**Prerequisite**: Complete C++ feature set (v1.0.0) before starting Rust port.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for how to get involved. Feature requests and bug reports are welcome via [GitHub Issues](https://github.com/DazzleTools/xormove/issues).

## Architecture Notes

### Flag Hierarchy

xormove uses a layered approach for configuration:

```
Environment → Auto-detection → Profile → Behavior → Explicit flags
     (lowest priority)                          (highest priority)
```

- **Environment** (`XMV_YES`, `XMV_PROFILE`): User defaults via shell profile
- **Auto-detection**: Based on file locations, disk space, existence
- **Profile** (`--profile`): Preset collection of defaults
- **Behavior** (`--behavior`): Operation type shorthand
- **Explicit flags** (`--1-to`, `--2-to`, `--yes`): Direct overrides

### Control Dimensions (v0.5.0+)

| Dimension | Flag | Values |
|-----------|------|--------|
| Operation type | `--behavior` | swap, encrypt, mv |
| File 1 destination | `--1-to` | REL, SAME, SAME-AS-2, path |
| File 2 destination | `--2-to` | REL, SAME, SAME-AS-1, path |
| File 1 action | `--1-action` | move, copy, preserve, delete |
| File 2 action | `--2-action` | move, copy, preserve, delete |
| Padding mode | `--pad` | loop, zero, one, truncate |
| Collision | `--on-exist` | overwrite, error, rename, prompt |

### Future: Globs and Folders

| Version | Capability |
|---------|------------|
| v0.5.0 | Single-file move, file-to-folder |
| v0.7.0 | Glob patterns (source only) |
| v0.8.0 | Folder operations |
| v0.9.0 | Recursive operations |
| v1.0.0 | Full mv compatibility |

## Design Documents

Analysis documents are maintained in `private/claude/` for major features:
- `2025-12-25__19-57-11__swap-behavior-disambiguation-design.md` - Swap behavior and profile architecture
- `2025-12-23__19-50-21__path-preservation-options-design.md` - Path preservation design
- `2025-12-23__15-56-13__xormove-roadmap-planning.md` - Roadmap planning analysis

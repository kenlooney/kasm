# kasm

[![CI](https://github.com/kenlooney/kasm/actions/workflows/ci.yml/badge.svg)](https://github.com/kenlooney/kasm/actions/workflows/ci.yml)

Ken's Assembler Project: an x86 assembler written in C. The assembler is at the
initial project-setup stage; instruction parsing and encoding are not implemented
yet.

## Requirements

- A C11 compiler (GCC, Clang, or MSVC)
- CMake 3.20 or newer

## Build and test

On Windows with Visual Studio 2026:

```sh
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

On Linux or WSL2 with GCC:

```sh
cmake --preset GCC-debug
cmake --build --preset GCC-debug
ctest --preset GCC-debug
```

The matching optimized presets are `windows-release` and `GCC-release`.

To make an optimized package:

```sh
cmake --preset windows-release
cmake --build --preset windows-release
cpack --config build/windows-release/CPackConfig.cmake -C Release
```

On Linux, replace `windows-release` with `GCC-release`. ZIP archives are written
to the selected build directory's `packages` subdirectory.

## Releases

Every merge or push to `main` automatically builds ZIP archives on Linux,
macOS, and Windows, creates the version tag, and publishes a GitHub release.
The tag comes from the version declared in the top-level `CMakeLists.txt`, so
increment that version before the next release. The workflow can also be run
manually from GitHub Actions to publish the current commit on `main`.

Package names use the format `kasm-v0.1.0-<platform>-<architecture>.zip`;
branch names are never included.

### Development snapshots

Every push to `dev` is built and tested on Linux, macOS, and Windows. Successful
builds are published as GitHub prereleases uniquely identified by tags such as
`v0.1.0-dev.42.a1b2c3d`. Snapshot ZIPs contain the same identifier, allowing a
contributor to download a binary or check out the exact source revision later.
After publishing, automation retains the newest 25 snapshots and deletes older
snapshot releases and their tags. Stable releases are never included in this
cleanup.

## License

Copyright (C) Kenneth Looney. This project is licensed under the
[GNU General Public License v3.0](LICENSE).

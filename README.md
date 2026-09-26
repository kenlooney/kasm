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

Pushing a tag such as `v0.1.0` builds ZIP archives on Linux, macOS, and Windows
and creates a GitHub release. The tag must match the version declared in the
top-level `CMakeLists.txt`.

## License

Copyright (C) Kenneth Looney. This project is licensed under the
[GNU General Public License v3.0](LICENSE).

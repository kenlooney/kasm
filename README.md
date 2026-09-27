# kasm

[![CI](https://github.com/kenlooney/kasm/actions/workflows/ci.yml/badge.svg)](https://github.com/kenlooney/kasm/actions/workflows/ci.yml)

Ken's Assembler Project: an x86 assembler written in C. The project currently
loads source files, tracks source positions, reports diagnostics, and tokenizes
the lexical elements documented below. Instruction parsing and machine-code
encoding are not implemented yet.

## Language support

The lexer currently recognizes the following source syntax. This section
describes lexical support only; recognized text is not yet assembled into x86
instructions.

### Identifiers

Identifiers begin with an ASCII letter or underscore and may continue with
ASCII letters, decimal digits, or underscores:

```text
[A-Za-z_][A-Za-z0-9_]*
```

Examples include `mov`, `eax_2`, and `_local`.

### Integer literals

| Format | Syntax | Examples |
| --- | --- | --- |
| Decimal | Decimal digits | `42`, `1_000_000` |
| Hexadecimal | `0x` or `0X`, then hexadecimal digits | `0xAD12`, `0xAD12_FFFF` |
| Binary | `0b` or `0B`, then binary digits | `0b1010`, `0b0000_1111` |

Underscores may separate digits for readability. They cannot appear first,
last, or consecutively. Integer values must fit in a signed `long long`
(`0` through `LLONG_MAX`). A leading minus sign is currently a separate token,
not part of the integer literal.

### Punctuation

The following single-character tokens are recognized:

```text
+ - * / % ( ) , ; { } :
```

### Whitespace and source locations

Spaces, tabs, carriage returns, and newlines separate tokens and are otherwise
ignored. Tokens and diagnostics use zero-based, half-open byte spans written as
`[start,end)`. Diagnostic line and column positions are one-based.

### Not implemented yet

- x86 instruction, register, and operand parsing
- Labels and symbol resolution (although `:` is tokenized)
- Comments, string literals, and character literals
- Machine-code encoding and object-file output

## Requirements

- A C11 compiler (GCC, Clang, or MSVC)
- CMake 3.20 or newer

## Build and test
### Test Example
```cmake
add_test(
    NAME kasm.cli.loads_source
    COMMAND kasm "${PROJECT_SOURCE_DIR}/examples/first.asm"
)

set_tests_properties(kasm.cli.loads_source PROPERTIES
    PASS_REGULAR_EXPRESSION "kasm [0-9]+\\.[0-9]+\\.[0-9]+"
)
```
Examples:
```powershell
ctest --test-dir build/windows-debug -C Debug -R "^kasm\.cli\.loads_source$" -V

cmake --build build/windows-debug --config Debug
ctest --test-dir build/windows-debug -C Debug -R "^kasm\.cursor\.cli$" -V
```

On Windows with Visual Studio 2026:

```sh
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug -V
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

## Version information in C

CMake generates `kasm/version.h` from the version in the top-level
`CMakeLists.txt`. Include it directly, or include `kasm/kasm.h`, to access:

```c
KASM_VERSION_MAJOR   /* numeric major version */
KASM_VERSION_MINOR   /* numeric minor version */
KASM_VERSION_PATCH   /* numeric patch version */
KASM_VERSION_STRING  /* complete string, such as "0.1.0" */
```

The `kasm_version()` function returns the same complete version string at
runtime.

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

#### Cleaning up Visual Studio Code of removed git tags
```powershell
git fetch origin --prune --prune-tags
```

## License

Copyright (C) Kenneth Looney. This project is licensed under the
[GNU General Public License v3.0](LICENSE).

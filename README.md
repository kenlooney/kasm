# kasm

[![CI](https://github.com/kenlooney/kasm/actions/workflows/ci.yml/badge.svg)](https://github.com/kenlooney/kasm/actions/workflows/ci.yml)

Ken's Assembler Project is an x86 assembler written in C. KASM currently
assembles a small 16-bit x86 language directly into flat binary files.
The project also includes KEMU, an early 16-bit emulator that can load and
execute the machine code produced by KASM.

> **v0.15.0 is KASM's first bootable release.** It can produce a complete
> 512-byte legacy BIOS boot sector containing executable real-mode code,
> padding, and the `55 AA` boot signature. The generated image boots
> successfully in QEMU.

## Assemble and boot the example

Configure, build, and test on Windows:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug -V
```

Assemble the included boot sector into a raw image:

```powershell
.\bin\Debug\kasm.exe .\examples\bootsect.asm .\bootsect.bin
```

Boot it with QEMU:

```powershell
qemu-system-i386 `
  -drive file=bootsect.bin,format=raw,if=floppy `
  -boot a `
  -no-reboot `
  -no-shutdown
```

The example intentionally enters a halt loop, so QEMU displays
`Booting from Floppy...` and remains running with a blank screen. This means
the BIOS accepted the image and transferred control to KASM's generated code.

## Execute the image with KEMU

KEMU is a small, deliberately partial 16-bit emulator and deterministic test
environment for KASM's output. Run the same boot image without launching a
full-system emulator:

```powershell
.\bin\Debug\kemu.exe .\bootsect.bin
```

Expected output:

```text
halted after 2 instructions at 0000:7C02, AX=0000
```

This demonstrates the complete local toolchain:

```text
bootsect.asm -> kasm -> bootsect.bin -> kemu
```

KEMU currently provides:

- 1 MiB of emulated physical memory;
- checked real-mode `segment:offset` address translation;
- image loading at physical address `0x7C00`;
- an initial `CS:IP` of `0000:7C00`;
- checked instruction fetching and a 1000-instruction execution limit;
- emulation of `mov ax, imm16`, `cli`, `sti`, `hlt`, and `jmp8`;
- clear failures for unknown opcodes, truncated instructions, oversized images,
  and execution that exceeds the step limit.

KEMU is not intended to replace QEMU or emulate an entire PC. It currently
implements only the machine state and instructions needed to test KASM's first
real-mode programs.

## Language support

KASM currently targets 16-bit x86 real mode and writes raw, little-endian
binary output. Source statements end with semicolons.

### Instructions

| Syntax | Encoding | Description |
| --- | --- | --- |
| `mov ax, expression;` | `B8 iw` | Load a 16-bit immediate into `AX` |
| `cli;` | `FA` | Clear the interrupt flag |
| `sti;` | `FB` | Set the interrupt flag |
| `hlt;` | `F4` | Halt the processor |
| `jmp8 label;` | `EB cb` | Explicit signed 8-bit relative jump |

`jmp8` targets must be defined labels within the range `-128..127` bytes from
the end of the jump instruction.

### Labels

Labels use an identifier followed by a colon:

```asm
hang:
hlt;
jmp8 hang;
```

KASM records statement offsets during layout and resolves jump targets before
encoding.

### Data and layout directives

| Syntax | Description |
| --- | --- |
| `org expression;` | Set the logical origin without emitting bytes |
| `db expression;` | Emit one byte |
| `dw expression;` | Emit a 16-bit little-endian word |
| `dd expression;` | Emit a 32-bit little-endian double word |
| `padto offset, byte;` | Emit the byte until output reaches the file offset |

`org` must appear before emitted content. `padto` cannot move backward, and its
fill value must be in the range `0..255`.

The bootable example is deliberately small:

```asm
org 0x7C00;

cli;
hang:
hlt;
jmp8 hang;

padto 510, 0;
dw 0xAA55;
```

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
last, or consecutively. Evaluated expressions must currently fit in the signed
32-bit range. A leading minus sign is a separate token, and unary operators are
not yet supported.

### Arithmetic expressions

Arithmetic expressions support integer literals, nested parentheses, and the
following binary operators:

| Precedence | Operators | Associativity |
| --- | --- | --- |
| Higher | `*`, `/`, `%` | Left-to-right |
| Lower | `+`, `-` | Left-to-right |

Parentheses override normal precedence. For example:

```text
9 * (3 + 2)
```

The current grammar is equivalent to:

```text
expression = product (("+" | "-") product)*
product    = primary (("*" | "/" | "%") primary)*
primary    = integer | "(" expression ")"
```

Expression nodes are stored in a dynamically growing indexed arena and are
evaluated before layout and semantic validation. Division or remainder by zero
is diagnosed as an error.

### Statement blocks

Statements may be visually grouped using nested braces:

```asm
{
    mov ax, 40 + 2;
    {
        hlt;
    }
}
```

Braces provide source organization and balance checking only. They do not
introduce scopes or namespaces. Statements from nested blocks are flattened
into the program in source order.

### Comments, whitespace, and source locations

Both `//` line comments and `/* ... */` block comments are supported. Block
comments may span multiple lines but do not nest; an unterminated block comment
is an error.

Spaces, tabs, carriage returns, and newlines separate tokens and are otherwise
ignored. Tokens and diagnostics use zero-based, half-open byte spans written as
`[start,end)`. Diagnostic line and column positions are one-based.

### Current limitations

- Only the 16-bit x86 real-mode target is implemented.
- `mov` currently supports only `AX` with a 16-bit immediate.
- `jmp8` is the only control-flow encoding and must be requested explicitly.
- Data directives currently accept one expression each.
- String literals, character literals, and unary expression operators are not
  implemented yet.
- Output is a flat binary image; object files and executable formats are not
  implemented yet.
- KEMU currently uses the legacy BIOS boot address and implements only KASM's
  initial instruction subset; devices, interrupts, and general PC hardware are
  not emulated yet.

## Requirements

- A C11 compiler (GCC, Clang, or MSVC)
- CMake 3.20 or newer
- QEMU (optional, for running the boot-sector example)

## Build and test

On Windows with Visual Studio 2026:

```powershell
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
KASM_VERSION_STRING  /* complete string, such as "0.15.0" */
```

The `kasm_version()` function returns the same complete version string at
runtime.

## Releases

Every merge or push to `main` automatically builds ZIP archives on Linux,
macOS, and Windows, creates the version tag, and publishes a GitHub release.
The tag comes from the version declared in the top-level `CMakeLists.txt`, so
increment that version before the next release. The workflow can also be run
manually from GitHub Actions to publish the current commit on `main`.

Package names use the format `kasm-v0.15.0-<platform>-<architecture>.zip`;
branch names are never included.

### Development snapshots

Every push to `dev` is built and tested on Linux, macOS, and Windows. Successful
builds are published as GitHub prereleases uniquely identified by tags such as
`v0.15.0-dev.42.a1b2c3d`. Snapshot ZIPs contain the same identifier, allowing a
contributor to download a binary or check out the exact source revision later.
After publishing, automation retains the newest 25 snapshots and deletes older
snapshot releases and their tags. Stable releases are never included in this
cleanup.

To remove locally cached tags that have been deleted remotely:

```powershell
git fetch origin --prune --prune-tags
```

## License

Copyright (C) Kenneth Looney. This project is licensed under the
[GNU General Public License v3.0](LICENSE).

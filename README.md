# kasm

[![CI](https://github.com/kenlooney/kasm/actions/workflows/ci.yml/badge.svg)](https://github.com/kenlooney/kasm/actions/workflows/ci.yml)

Ken's Assembler Project is an x86 assembler written in C. KASM currently
assembles a small 16-bit x86 language directly into flat binary files.
The project also includes KEMU, an early 16-bit emulator that can load and
execute the machine code produced by KASM.

> **v0.24.0 establishes a complete real-mode startup environment.** KASM can
> normalize `CS` with an immediate far jump, initialize `DS`, `ES`, `SS`, and
> `SP`, and emit a complete 512-byte startup image. KEMU checks far-jump
> targets against the loaded image, and the same image boots under QEMU.
>
> **v0.22.0 added the first segment-register MOV form.** KASM can encode
> `mov es, ax`, `mov ss, ax`, and `mov ds, ax` with opcode `8E`, and KEMU can
> decode and execute their register-mode ModR/M bytes.
>
> **v0.20.0 completes KASM's first no-operand, single-byte instruction
> milestone.** KASM and KEMU now share an initial real-mode instruction set
> covering processor control, flag manipulation, sign extension, stack-based
> flag save/restore, and interrupt returns.
>
> **v0.15.0 was KASM's first bootable release.** KASM can produce a complete
> 512-byte legacy BIOS boot sector containing executable real-mode code,
> padding, and the `55 AA` boot signature.

## Assemble and boot the example

Configure, build, and test on Windows:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug -V
```

Assemble the complete startup sector into a raw image:

```powershell
.\bin\Debug\kasm.exe .\examples\startup.asm .\startup.bin
```

Boot it with QEMU:

```powershell
qemu-system-x86_64.exe `
  -drive format=raw,file=.\startup.bin `
  -no-reboot `
  -no-shutdown
```

The example intentionally enters a halt loop, so QEMU displays
`Booting from Hard Disk...` and remains running with a blank screen. This means
the BIOS accepted the image and transferred control to KASM's generated code.

## Execute the image with KEMU

KEMU is a small, deliberately partial 16-bit emulator and deterministic test
environment for KASM's output. Run the same boot image without launching a
full-system emulator:

```powershell
.\bin\Debug\kemu.exe .\startup.bin
```

Expected output:

```text
halted after 9 instructions at 0000:7C14, AX=0000
```

This demonstrates the complete local toolchain:

```text
startup.asm -> kasm -> startup.bin -> kemu or QEMU
```

KEMU currently provides:

- 1 MiB of emulated physical memory;
- checked real-mode `segment:offset` address translation;
- image loading at physical address `0x7C00`;
- an initial `CS:IP` of `0000:7C00`;
- checked instruction fetching and a 1000-instruction execution limit;
- emulation of encoded-register instructions, segment-register MOV, `jmp8`,
  immediate far jumps, and the no-operand instruction set documented below;
- a 16-bit downward-growing stack used by `push`, `pop`, `pushf`, `popf`, and
  `iret`;
- modeled 16-bit FLAGS state, including carry, interrupt, direction, and
  arithmetic status flags;
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
| `inc register;` | `40+rw` | Increment a 16-bit register |
| `dec register;` | `48+rw` | Decrement a 16-bit register |
| `push register;` | `50+rw` | Push a 16-bit register onto the stack |
| `pop register;` | `58+rw` | Pop a stack value into a 16-bit register |
| `xchg ax, register;` | `90+rw` | Exchange `AX` with a 16-bit register |
| `mov register, expression;` | `B8+rw iw` | Load a 16-bit immediate into a register |
| `mov segment, ax;` | `8E /r` | Load `ES`, `SS`, or `DS` from `AX` |
| `nop;` | `90` | Perform no operation |
| `cli;` | `FA` | Clear the interrupt flag |
| `sti;` | `FB` | Set the interrupt flag |
| `hlt;` | `F4` | Halt the processor |
| `clc;` | `F8` | Clear the carry flag |
| `stc;` | `F9` | Set the carry flag |
| `cmc;` | `F5` | Complement the carry flag |
| `cld;` | `FC` | Clear the direction flag |
| `std;` | `FD` | Set the direction flag |
| `lahf;` | `9F` | Load status flags into `AH` |
| `sahf;` | `9E` | Store `AH` into the status flags |
| `pushf;` | `9C` | Push FLAGS onto the stack |
| `popf;` | `9D` | Restore FLAGS from the stack |
| `cbw;` | `98` | Sign-extend `AL` into `AX` |
| `cwd;` | `99` | Sign-extend `AX` into `DX:AX` |
| `iret;` | `CF` | Restore `IP`, `CS`, and FLAGS from the stack |
| `jmp8 label;` | `EB cb` | Explicit signed 8-bit relative jump |
| `jmpfar segment, label;` | `EA ptr16:16` | Jump to an absolute 16-bit segment and offset |

`jmp8` targets must be defined labels within the range `-128..127` bytes from
the end of the jump instruction. A `jmpfar` segment and its origin-adjusted
label offset must each fit in 16 bits.

The supported 16-bit registers are `ax`, `cx`, `dx`, `bx`, `sp`, `bp`, `si`,
and `di`. In the encoding table, `rw` selects one of these registers and `iw`
is a 16-bit immediate stored in little-endian byte order. The compact `xchg`
form requires `ax` as its first operand.

The supported segment-register destinations are `es`, `ss`, and `ds`. This
initial `8E` form accepts only `ax` as its source; `cs`, `fs`, `gs`, other
general-purpose source registers, and memory operands are rejected.

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

The complete startup example in `examples/startup.asm` normalizes `CS`,
initializes the data and stack segments, and installs a stack before entering
its halt loop:

```asm
org 0x7C00;
jmpfar 0x0000, normalized;
normalized:
cli;
mov ax, 0;
mov ds, ax;
mov es, ax;
mov ss, ax;
mov sp, 0x7C00;
sti;
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
- General-purpose register operands are limited to the eight 16-bit registers;
  segment-register MOV currently supports only `es`, `ss`, and `ds` from `ax`.
- Control flow is currently limited to explicit `jmp8` and `jmpfar` forms.
- Data directives currently accept one expression each.
- String literals, character literals, and unary expression operators are not
  implemented yet.
- Output is a flat binary image; object files and executable formats are not
  implemented yet.
- KEMU currently uses the legacy BIOS boot address and implements only KASM's
  initial instruction subset. `iret` can restore a prepared interrupt frame,
  but interrupt delivery, devices, and general PC hardware are not emulated
  yet.

### Future output-format ladder

KASM currently emits only flat binary images. A future `format` directive may
grow the output pipeline in stages:

```asm
format raw;      /* BIOS boot sectors, loaders, and kernel blobs */
format elf64;    /* sectioned kernels and other ELF images */
format pe64;     /* PE32+ executables */
format efi;      /* PE32+ image with the EFI application subsystem */
```

The first incremental step can make `format raw;` explicitly select KASM's
existing emitter while rejecting unimplemented formats clearly. The ELF and
PE stages will require sections, headers, symbols, and eventually relocation
support. `format efi;` is intended as an OS-development convenience shorthand
for the appropriate PE32+ container and EFI subsystem rather than as an
unrelated executable format. Variants such as `elf32` and `pe32` can be added
when KASM supports their corresponding target modes.

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

### VS Code debugging

The included VS Code launch configuration can build and debug KASM using the
currently active assembly source file. Set a breakpoint in the C source, make
the `.asm` file the active editor, select **Debug KASM with active ASM file**,
and press F5.

The **Debug KEMU with segment MOV image** configuration builds KASM and KEMU,
assembles `examples/segment_mov.asm` as `build/vscode-segment-mov.bin`, and
launches that image in KEMU. This provides a focused way to trace opcode `8E`
and its ModR/M byte.

The **Debug KEMU with startup image** configuration launches the startup image
created by `kasm.startup.cli`. Run that focused test once to create
`build/windows-debug/tests/startup.bin`, then use the configuration to inspect
the normalized `CS:IP`, initialized segment registers, and stack pointer.

> **Future enhancement:** add a similar QEMU launch configuration that reuses
> a named assembled image, making it easy to inspect the same output in a
> full-system emulator.

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
KASM_VERSION_STRING  /* complete string, such as "0.24.0" */
```

The `kasm_version()` function returns the same complete version string at
runtime.

## Releases

Every merge or push to `main` automatically builds ZIP archives on Linux,
macOS, and Windows, creates the version tag, and publishes a GitHub release.
The tag comes from the version declared in the top-level `CMakeLists.txt`, so
increment that version before the next release. The workflow can also be run
manually from GitHub Actions to publish the current commit on `main`.

Package names use the format `kasm-v0.24.0-<platform>-<architecture>.zip`;
branch names are never included.

### Development snapshots

Every push to `dev` is built and tested on Linux, macOS, and Windows. Successful
builds are published as GitHub prereleases uniquely identified by tags such as
`v0.24.0-dev.43.a1b2c3d`. Snapshot ZIPs contain the same identifier, allowing a
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

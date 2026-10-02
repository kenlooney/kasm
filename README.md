# kasm

[![CI](https://github.com/kenlooney/kasm/actions/workflows/ci.yml/badge.svg)](https://github.com/kenlooney/kasm/actions/workflows/ci.yml)

Ken's Assembler Project is an x86 assembler written in C. KASM emits raw,
little-endian flat binaries. Its current end-to-end target is a 16-bit BIOS
boot sector, with early source-level support for assembling selected 32-bit
instructions for a future protected-mode transition. The project also
includes KEMU, a deliberately partial 16-bit emulator for testing supported
machine code.

> **v0.29.0 adds software-interrupt encoding and a BIOS disk-loading
> milestone.** KASM emits `INT imm8` (`CD ib`), and KEMU provides a limited
> `INT 10h/AH=0Eh` teletype shim. The Windows debug suite passes all 38 tests.
> In a manual QEMU/SeaBIOS check, the two-sector disk example read its second
> sector with `INT 13h`, continued past `JC`, and printed `B` from the loaded
> second stage. This is still real-mode code, not a protected-mode OS yet.

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

The current `dev` branch builds on that real-mode foundation with source-level
encoding-mode regions, label expressions, named `equ` constants, and
carry-conditional short jumps, plus `int` encoding. The project version in
`CMakeLists.txt` is currently `0.29.0`; consult the release tags to see which development features
have shipped in a stable release.

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
- emulation of the supported 16-bit instructions documented below, including
  `jc`/`jb` and `jnc`/`jae` conditional branches;
- a 16-bit downward-growing stack used by `push`, `pop`, `pushf`, `popf`, and
  `iret`;
- modeled 16-bit FLAGS state, including carry, interrupt, direction, and
  arithmetic status flags;
- clear failures for unknown opcodes, truncated instructions, oversized images,
  and execution that exceeds the step limit.

KEMU is not intended to replace QEMU or emulate an entire PC. It currently
implements only the machine state and instructions needed to test KASM's
real-mode programs. Its `INT 10h/AH=0Eh` shim writes `AL` to standard output;
it does not model an interrupt-vector-table lookup, interrupt stack frame, or
BIOS ROM. Other vectors and video functions fail explicitly. `iret` can only
restore an interrupt frame that a program has already placed on the stack.

### BIOS teletype and second-stage loading

Assemble and execute the teletype example:

```powershell
.\bin\Debug\kasm.exe .\examples\int.asm .\int.bin
.\bin\Debug\kemu.exe .\int.bin
```

Expected output:

```text
Ahalted after 5 instructions at 0000:7C0A, AX=0E41
```

`AX=0E41h` selects BIOS teletype function `AH=0Eh` and character `AL=41h`
(`A`). The 512-byte image begins with `B8 41 0E BB 07 00 CD 10 FA F4` and
ends with the BIOS signature `55 AA`.

The [disk-loading example](examples/int_disk.asm) goes further: it creates a
1024-byte image, reads CHS sector two from drive `80h` into `0000:7E00`,
checks Carry Flag with `jc disk_error;`, and jumps to the loaded second stage:

```powershell
.\bin\Debug\kasm.exe .\examples\int_disk.asm .\int_disk.img
qemu-system-x86_64.exe `
  -drive format=raw,file=.\int_disk.img,index=0,media=disk `
  -boot order=c -no-reboot -no-shutdown
```

The successful manual SeaBIOS check displayed `B`. The error path prints `E`;
that path has not yet been manually confirmed in this milestone. This small
fixture assumes boot drive `80h`; it is not a general-purpose disk loader.
KEMU does not implement `INT 13h`, so run this disk example under QEMU rather
than KEMU. The next OS milestone is the GDT and actual protected-mode
transition, followed later by a 64-bit long-mode path.

## Language support

KASM writes raw, little-endian x86 binary output. Source statements end with
semicolons. The working boot path is 16-bit real mode. `mode` directives
select assembler encoding rules for source regions; they do not switch the
running processor into another mode. KEMU remains a 16-bit emulator, so it
cannot execute the assembler's 32-bit output.

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
| `int expression;` | `CD ib` | Invoke the interrupt vector numbered `0..255` |
| `jmp8 label;` | `EB cb` | Explicit signed 8-bit relative jump |
| `jc label;` or `jb label;` | `72 cb` | Jump when Carry Flag is set |
| `jnc label;` or `jae label;` | `73 cb` | Jump when Carry Flag is clear |
| `jmpfar segment, label;` | `EA ptr16:16` | Jump to an absolute 16-bit segment and offset |

`jmp8`, `jc`/`jb`, and `jnc`/`jae` targets must be defined labels within the
range `-128..127` bytes from the end of the jump instruction. `jb` and `jc`
are two source spellings for the same condition and opcode; likewise, `jae`
and `jnc` are aliases for the same condition and opcode. They are aliases,
not four distinct instructions: `jb`/`jc` both encode as `72 cb`, while
`jae`/`jnc` both encode as `73 cb`. A `jmpfar` segment and its origin-adjusted
label offset must each fit in 16 bits.

`int` accepts a vector expression that resolves to `0..255`; `ib` is its
one-byte immediate. KASM permits this encoding in all three source encoding
modes, but that does not make legacy BIOS services available outside real
mode. KEMU supports only the teletype shim described above, not general
interrupt delivery or BIOS disk services.

The supported 16-bit registers are `ax`, `cx`, `dx`, `bx`, `sp`, `bp`, `si`,
and `di`. In the encoding table, `rw` selects one of these registers and `iw`
is a 16-bit immediate stored in little-endian byte order. The compact `xchg`
form requires `ax` as its first operand.

The supported segment-register destinations are `es`, `ss`, and `ds`. This
initial `8E` form accepts only `ax` as its source; `cs`, `fs`, `gs`, other
general-purpose source registers, and memory operands are rejected.

### Encoding-mode regions

Use `mode` directives to tell KASM which encoding rules apply to following
statements:

```asm
mode 16;
mov ax, 0x1234;
mode 32;
mov eax, 0x12345678;
mode 16;
mov cx, 0x5678;
```

The directives emit no bytes. In MODE_16, `mov r16, imm16` uses `B8+rw iw`;
in MODE_32, the currently implemented `mov r32, imm32` form uses `B8+rd id`.
The current assembler does not yet support 64-bit MOV encodings; `INT imm8`
uses the same two-byte encoding in all three source modes. Most other
instructions remain MODE_16-only, and
KEMU does not execute MODE_32 code. A `mode 32;` directive is not a CPU mode
transition; the code that changes processor mode is a separate part of the
boot process.

### Labels

Labels use an identifier followed by a colon:

```asm
hang:
hlt;
jmp8 hang;
```

KASM records statement offsets during layout and resolves jump targets before
encoding. Labels can also participate in expressions, for example
`dw end - start;`, and can be assigned a reusable name with `equ`:

```asm
start:
db 0;
end:
span equ end - start;
dw span;
```

`equ` emits no bytes. EQU expressions can use literals, arithmetic, and
labels, but cannot currently refer to another EQU constant.

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

Arithmetic expressions support integer literals, labels, nested parentheses,
and the following binary operators:

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
primary    = integer | identifier | "(" expression ")"
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

- Raw flat-binary output is the only output format. The complete boot path is
  16-bit real mode; MODE_32 supports `mov r32, imm32` and `int imm8` encoding.
  MODE_64 supports `int imm8`, but not 64-bit MOV or a long-mode boot path.
- General-purpose register operands are limited to the eight 16-bit registers;
  the implemented 32-bit MOV-immediate form supports the eight 32-bit general-
  purpose registers. Segment-register MOV supports only `es`, `ss`, and `ds`
  from `ax`.
- Control flow includes `jmp8`, `jmpfar`, `jc`/`jb`, and `jnc`/`jae`; other
  conditional conditions and near calls/jumps are not implemented.
- Data directives currently accept one expression each.
- String literals, character literals, and unary expression operators are not
  implemented yet.
- KEMU executes only its supported 16-bit subset, with a host shim for
  `INT 10h/AH=0Eh`. General interrupt delivery, `INT 13h` disk services,
  devices, and general PC hardware are not emulated; `iret` can restore a
  prepared interrupt frame.
- Object files and executable formats are not implemented yet.

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

At the `0.29.0` development milestone, all 38 Windows debug tests pass,
including INT image assembly and KEMU teletype execution. Run that focused
pair with:

```powershell
ctest --preset windows-debug -R "kasm\.int|kemu\.int" -V
```

The QEMU/SeaBIOS disk-read result is a separate manual integration check,
not part of that automated suite.

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
KASM_VERSION_STRING  /* complete string, such as "0.29.0" */
```

The `kasm_version()` function returns the same complete version string at
runtime.

## Releases

Every merge or push to `main` automatically builds ZIP archives on Linux,
macOS, and Windows, creates the version tag, and publishes a GitHub release.
The tag comes from the version declared in the top-level `CMakeLists.txt`, so
increment that version before the next release. The workflow can also be run
manually from GitHub Actions to publish the current commit on `main`.

Package names use the format
`kasm-v<version>-<platform>-<architecture>.zip`; branch names are never
included.

### Development snapshots

Every push to `dev` is built and tested on Linux, macOS, and Windows. Successful
builds are published as GitHub prereleases uniquely identified by tags such as
`v0.29.0-dev.53.a1b2c3d`. Snapshot ZIPs contain the same identifier, allowing a
contributor to download a binary or check out the exact source revision later.
After publishing, automation retains the newest four snapshots and deletes
older snapshot releases and their tags. The stable release workflow separately
retains the newest three version-tagged releases and deletes older releases and
their tags. Together, the workflows keep up to seven project-generated GitHub
releases: three stable releases and four development snapshots. Manually
created releases whose tags do not match the workflows' version-tag patterns
are not included in this retention policy.

To remove locally cached tags that have been deleted remotely:

```powershell
git fetch origin --prune --prune-tags
```
## Project Snapshot Ratings

These are subjective progress scores, not claims of full x86 compatibility or
production readiness. Each new snapshot should be added to the table rather
than replacing an earlier score. Use the same five areas, each worth two
points, so changes are easier to compare:

- Assembler pipeline and encoding coverage
- Progress toward the BIOS-to-protected-mode OS goal
- Emulator behavior and automated validation
- Architecture and maintainability
- Documentation, build, and release workflow

| Snapshot | Score | Notes |
| --- | ---: | --- |
| 2026-10-02, `dev` at `93c51d7`, project version `0.29.0` | **8.9 / 10** | INT encoding, a scoped KEMU teletype shim, 38 passing Windows tests, and a manual SeaBIOS second-stage disk-load success; general interrupt delivery and protected-/long-mode transitions remain future work. |
| 2026-10-02, `dev` at `3463748`, project version `0.28.0` | **8.7 / 10** | Strong staged assembler pipeline, real-mode boot path, and passing Windows test suite; BIOS interrupt delivery and the 32-bit/64-bit OS transitions remain future work. |
| Earlier README rating, snapshot not recorded | 9.1 / 10 | Kept for history; it had no pinned revision or consistent rubric, so it is not directly comparable to the new baseline. |

The `0.28.0` entry is the first score using this rubric. Each entry identifies
the code revision assessed; the associated README update is a subsequent
documentation-only change. The `0.29.0` increase reflects the newly tested
instruction and real BIOS disk-loading progress, not full BIOS emulation.

## License

Copyright (C) Kenneth Looney. This project is licensed under the
[GNU General Public License v3.0](LICENSE).

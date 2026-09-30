# KASM Agent Guide

## Project purpose

KASM is Kenneth Looney's learning project: a small x86 assembler written in C,
plus KEMU, a deliberately partial 16-bit emulator for executing KASM's output.
The long-term idea may include multiple ISAs and runtime backend plugins, but
the current focus is a correct, understandable 16-bit x86 implementation.

The user is intentionally implementing many features personally to learn the
entire pipeline. Do not take over an implementation merely because it would be
faster. When asked to review, inspect and explain what is correct or missing;
do not modify files unless the user also asks for a fix. When asked to help with
a particular step, make the smallest focused change and explain why it works.

## Current architecture

- `kasm_core` is the reusable static assembler library.
- `kasm` is the assembler CLI built from `src/main.c`.
- `kemu` is the emulator CLI built from `src/kemu.c`.
- `src/main.c` and `src/kemu.c` must remain excluded from `kasm_core`; otherwise
  multiple `main` definitions will be linked into consumers.
- User-facing executables are written to `bin/<configuration>` (for example,
  `bin/Debug/kasm.exe` and `bin/Debug/kemu.exe`). Test executables remain in the
  CMake build tree.
- Both CLIs are installed and included in release packages.

Generated files in `build/`, `bin/`, `install/`, and `*.bin` are ignored. Do not
commit or manually edit generated CMake projects, binaries, or test images.

## Assembler pipeline

An instruction is not complete until it passes through every stage:

```text
source text
-> parser statement kind
-> semantic validation
-> layout size
-> emitted bytes
-> KEMU behavior
-> tests
```

Relevant files are:

- Statement model/API: `include/kasm/program.h`
- Parsing: `src/program.c`
- Expression evaluation and semantic checks: `src/semantic.c`
- Byte layout and statement offsets: `src/layout.c`
- Machine-code emission: `src/encode.c`
- Byte buffer and endian helpers: `src/bytes.c`, `src/emit.c`
- Emulator: `src/kemu.c`
- Instruction checklist: `docs/instruction-roadmap.md`

Use an authoritative x86 reference for opcode values and CPU-generation
requirements. Do not rely on memory for encodings.

## Current feature set

The current target is 16-bit x86 real mode with little-endian flat-binary
output. At the time this guide was added, the implemented instruction subset
includes:

- `mov ax, imm16` (`B8 iw`)
- `cli` (`FA`)
- `sti` (`FB`)
- `hlt` (`F4`)
- `nop` (`90`)
- `jmp8 label` (`EB cb`)

Supported data/layout directives include `org`, `db`, `dw`, `dd`, and
`padto offset, fill`. Labels and signed short-jump resolution are implemented.
Expressions support integer arithmetic and parentheses.

Treat `docs/instruction-roadmap.md` as the checklist, but verify the code and
tests because the roadmap can temporarily lag behind work in progress. Mark an
instruction complete only after its test coverage is updated.

## KEMU model

KEMU currently models:

- 1 MiB of physical memory;
- checked real-mode `segment * 16 + offset` translation;
- loading a raw image at physical `0x7C00`;
- normalized entry state `CS:IP = 0000:7C00`;
- checked fetches limited to the loaded image;
- a bounded execution loop;
- the same initial opcode subset emitted by KASM.

The fixed boot address is acceptable for the current learning version. A
future generalized version may introduce configurable machine profiles and
entry addresses. Do not prematurely redesign it into a multi-ISA plugin system
unless the user explicitly asks.

`HLT` records halted CPU state and ends the current driver loop. With the
current boot example, instructions after `HLT` are not executed. `PAUSE` is a
different, two-byte spin-loop hint (`F3 90`) and is not currently implemented.

## Boot-sector integration fixture

`examples/bootsect.asm` is the end-to-end fixture. With the current `NOP`, its
initial bytes are:

```text
90 FA F4 EB FD
```

The image is padded from offset 5 through 509 and ends with the little-endian
BIOS signature `55 AA` at offsets 510 and 511. Its total size is 512 bytes.

Adding an instruction before `padto` shifts the expected prefix and the start
of the zero-padding assertion in `tests/bootsect_cli.c`. A failure such as
"expected zero padding ... got <opcode>" usually means the test fixture's
padding boundary was not updated.

The current KEMU result for the boot fixture is expected to resemble:

```text
halted after 3 instructions at 0000:7C03, AX=0000
```

## Building and testing

On Windows, run from the repository root:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug -V
```

For concise failure output:

```powershell
ctest --preset windows-debug --output-on-failure
```

For the focused boot-sector test:

```powershell
ctest --preset windows-debug -R "^kasm\.bootsect\.cli$" -V
```

Manual end-to-end use:

```powershell
.\bin\Debug\kasm.exe .\examples\bootsect.asm .\bootsect.bin
.\bin\Debug\kemu.exe .\bootsect.bin
```

Warnings are treated as errors in the Windows debug preset. Keep MSVC `/W4`
and GCC/Clang `-Wall -Wextra -Wpedantic` builds clean. After code changes, run
the focused relevant test and then the full suite.

## CMake and release cautions

- Preserve the configured runtime output directory. The user explicitly wants
  runnable products in `bin/<configuration>`.
- When adding another CLI entry point, exclude it from the glob used by
  `kasm_core`, create a distinct executable target, apply matching warning and
  MSVC runtime settings, and install it if it belongs in packages.
- `main` is the stable-release branch; pushing/merging there triggers release
  automation. `dev` produces development snapshots.
- Confirm the version in the top-level `CMakeLists.txt` before a release. Do
  not push, merge, tag, rewrite history, or publish releases unless explicitly
  requested.
- Preserve user changes in the dirty worktree and avoid unrelated cleanup.

## Collaboration style

The user is enthusiastic and learns by building features incrementally. Match
that energy while remaining technically precise. Explain byte offsets,
endianness, instruction-pointer movement, and test failures concretely. Point
out mistakes without implying the entire implementation is wrong when only a
fixture or assertion needs updating.

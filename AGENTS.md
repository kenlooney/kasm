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

The current release is **v0.24.0**. The `main` and `dev` branches both point to
the tested release merge `3fd4726`. GitHub Actions successfully built and
published the release, and the downloaded Windows AMD64 package was used to
assemble `examples/startup.asm`; that generated image booted successfully
under QEMU/SeaBIOS.

The current target remains 16-bit x86 real mode with little-endian flat-binary
output. Implemented instruction families include:

- all eight compact register forms of `INC`, `DEC`, `PUSH`, and `POP`;
- compact `XCHG AX,r16`;
- `MOV r16,imm16` for AX, CX, DX, BX, SP, BP, SI, and DI;
- `MOV ES/SS/DS,AX` through register-mode opcode `8E /r`;
- `NOP`, `CLI`, `STI`, `HLT`, `CLC`, `STC`, `CMC`, `CLD`, and `STD`;
- `LAHF`, `SAHF`, `PUSHF`, `POPF`, `CBW`, `CWD`, and `IRET`;
- signed short `jmp8 label` (`EB cb`);
- immediate far `jmpfar segment, label` (`EA ptr16:16`).

Supported data/layout directives include `org`, `db`, `dw`, `dd`, and
`padto offset, fill`. Labels and signed short-jump resolution are implemented.
Far-jump resolution adds the program origin to the destination label's file
offset, and requires both the resulting offset and segment to fit 16 bits.
Expressions support integer arithmetic and parentheses.

The next planned architectural change is Chapter 22: source-level encoding
mode regions. Each statement will eventually remember whether it belongs to a
16-, 32-, or 64-bit region. A `mode` directive will select assembler encoding
rules; it must not be confused with an instruction that changes the running
CPU mode. The book chapter at
`C:\Users\kenne\books\assemblerbook\src\os_22_mode_regions.md` is still in
breadcrumb/checklist form and should be rewritten as a beginner-friendly
walkthrough before the user implements it.

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
- all currently implemented KASM instructions listed above;
- checked `EA` far jumps that reject truncated operands and destinations
  outside the loaded image.

The fixed boot address is acceptable for the current learning version. A
future generalized version may introduce configurable machine profiles and
entry addresses. Do not prematurely redesign it into a multi-ISA plugin system
unless the user explicitly asks.

`HLT` records halted CPU state and ends the current driver loop. With the
current boot example, instructions after `HLT` are not executed. `PAUSE` is a
different, two-byte spin-loop hint (`F3 90`) and is not currently implemented.

## Startup integration fixture

`examples/startup.asm` is the primary v0.24.0 end-to-end fixture:

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

Its 22-byte executable prefix is:

```text
EA 05 7C 00 00 FA B8 00 00 8E D8 8E C0 8E D0
BC 00 7C FB F4 EB FD
```

The image is padded with zeroes from offset 22 through 509 and ends with the
little-endian BIOS signature `55 AA` at offsets 510 and 511. Its total size is
512 bytes. `tests/startup_cli.c` checks the complete prefix, padding, signature,
and total size.

Adding an instruction before `padto` shifts the expected prefix and the start
of the zero-padding assertion in `tests/startup_cli.c`. A failure such as
"expected zero padding ... got <opcode>" usually means the test fixture's
padding boundary was not updated.

The current KEMU result for the startup fixture is:

```text
halted after 9 instructions at 0000:7C14, AX=0000
```

The instruction after HLT does not execute in KEMU because its driver stops on
the halted state. Under QEMU, hardware interrupts can wake HLT and `jmp8 hang`
returns execution to it. A blank QEMU display after `Booting from Hard Disk...`
is expected because this image does not write to a display yet.

The older `examples/bootsect.asm` fixture remains as a smaller regression test.

## Far-jump coverage

The far-jump tests deliberately cover more than the startup image:

- `far_jump_pair.asm` uses the nontrivial pair `0700:0C05`, whose physical
  address is `0x7C05`;
- `far_jump_normalize.asm` verifies the real `0000:7C05` normalization jump;
- `far_jump_truncated.asm` requires a clear truncated-instruction failure;
- `far_jump_outside.asm` requires a clear out-of-image target failure.

Negative assembler tests cover an undefined label, a segment outside 16 bits,
a destination outside 16 bits, and a missing comma. The roadmap still honestly
marks automated nonzero-AX execution coverage for segment MOV as unfinished;
zero-valued startup state is not strong enough to prove those assignments.

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

For the focused startup test:

```powershell
ctest --preset windows-debug -R "^kasm\.startup\.cli$" -V
```

For all segment-MOV, far-jump, and startup checks:

```powershell
ctest --preset windows-debug -R "startup|segment|far" --output-on-failure
```

Manual end-to-end use:

```powershell
.\bin\Debug\kasm.exe .\examples\startup.asm .\startup.bin
.\bin\Debug\kemu.exe .\startup.bin
qemu-system-x86_64.exe -drive format=raw,file=.\startup.bin -no-reboot -no-shutdown
```

The Windows debug suite currently contains 29 tests. At the v0.24.0 release
checkpoint, all 29 passed.

## VS Code debugging

The checked-in `.vscode` configuration contains three useful launch entries:

- **Debug KASM with active ASM file** builds KASM and assembles whichever
  `.asm` file is active in the editor;
- **Debug KEMU with segment MOV image** assembles the focused segment-MOV
  fixture and traces opcode `8E`;
- **Debug KEMU with startup image** runs
  `build/windows-debug/tests/startup.bin`, which must first be generated by
  `kasm.startup.cli`.

VS Code's C++ debugger does not provide a global hexadecimal-display toggle in
the Locals menu. Add values to Watch and use format suffixes, for example:

```text
(unsigned int)opcode,x
(unsigned int)modrm,x
(unsigned int)cpu->ip,x
(unsigned int)cpu->sp,x
```

At the startup HLT case, useful expected values are `CS=0000`, `IP=7C14`,
`DS=ES=SS=0000`, and `SP=7C00`.

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
- v0.24.0 has already been released. Increment the top-level project version
  before the next release rather than rebuilding or replacing the v0.24.0 tag.
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

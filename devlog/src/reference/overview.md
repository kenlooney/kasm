# Overview

Kasm 0.37.0 (documentation-only) is a small C assembler with a limited x86-64
instruction set and initial 16-bit MOV/ADD/SUB support. It owns source text,
lexes tokens, parses expressions and statements, validates operands, assigns
image offsets, resolves labels and relocations, emits machine-code bytes, and
can decode supported instruction images back into a listing.

The project also emits raw data directives and assembly-time repetition. The
default target is hosted x86-64 code; `--bits 16` selects the developing
16-bit encoding path, while `--bits 32` remains rejected. A raw data image is
not automatically a bootable program. Kasm can also generate Windows x64 COFF
objects containing one instruction-only `.text` section and an exported
label. Kasm also writes a minimal standalone Linux x86-64 ELF executable with
one instruction-only read/execute segment and an explicit entry label.

## COFF output

COFF output requires `-o <output-path>`. To choose the filename and directory:

```powershell
.\build\windows-debug\Debug\kasm.exe --format coff --export answer -o .\build\coff_answer.obj examples/coff_answer.asm
```

The output directory must already exist; quote paths containing spaces. The
export must name a label with instructions after it. Raw output remains the
default. The COFF writer uses dynamically allocated storage with the existing
16 MiB image limit, supports long symbol names, and accepts REL32 and ADDR64
relocation records. Source-level external declarations and separate data
sections remain future frontend work. Existing load-time absolute-jump
patches are rejected for object output because they require conversion into
linker relocations.

Raw output uses the source filename with its last extension replaced: for
example, `examples/coff_answer.asm` writes `coff_answer.bin` and
`coff_answer.h` in the current working directory. The runner and inspector
examples default to `program.h` from `program.asm`; define
`KASM_GENERATED_HEADER` when using another generated header.

## ELF executable output

```powershell
.\build\windows-debug\Debug\kasm.exe --format elf --entry _start -o .\build\exit42 examples/exit42.asm
```

The first ELF profile has no section table, interpreter, or dynamic
dependencies. It rejects data and load-time patches. See
[Executable output](executable-formats.md) for its layout and Linux execution
steps.

See [Using a release](releases.md) to try Kasm without a source checkout, or
[Building and testing](building-and-testing.md) to build it from source.

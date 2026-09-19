# 0.33.0 — COFF Objects and Linking with the Real World

Every prior version produced a **raw image**: bytes meant to be copied into
executable memory directly by a hand-written runner. Version 0.33.0 is the
first time Kasm's output is meant to be consumed by someone else's tool — a
linker.

## Why an object format, and why COFF first

`5091655 — Implement COFF support and enhance expression handling` added
`--format coff --export <label> -o <output-path>`, producing a Windows x64
COFF object: symbol and string tables, one instruction-only `.text` section,
one exported label, and both REL32 and ADDR64 relocation records. COFF was
the natural first choice on a project developed and tested primarily on
Windows, and it reuses infrastructure the project already trusted — the same
16 MiB image-size ceiling (`KASM_IMAGE_LIMIT`) applies to COFF output as to
raw output.

```powershell
.\build\windows-debug\Debug\kasm.exe --format coff --export answer -o .\build\coff_answer.obj examples\coff_answer.asm
```

## Two things that had to stop being implicit

Two decisions in this milestone are direct descendants of the "make
implicit assumptions explicit" pattern seen at 0.22.0 and 0.26.0:

- **Load-time absolute jumps (`jmp abs`) are rejected for object output.**
  The 0.11.0/0.12.0 relocation mechanism patches a runtime address directly
  into a loaded image; a linker-resolved object needs a *linker* relocation
  record instead, which is a different mechanism entirely. Rather than
  silently emitting an incorrect or unusable relocation, object output
  explicitly rejects `jmp abs` until it can be converted into a proper
  linker relocation.
- **Raw output file naming changed.** Previously fixed as `program.bin` /
  `generated.h` regardless of input filename, raw output now derives its
  name from the source file — `examples/coff_answer.asm` now produces
  `coff_answer.bin` and `coff_answer.h`. This matters once a single project
  might assemble more than one source file into more than one object; fixed
  output names stop being safe once linking multiple objects together is a
  realistic workflow.

## Data directives learn about labels

This milestone also let data directives resolve **label expressions** after
layout, not just constants and `$`/`$$` location expressions. Constant-only
expressions still go through the overflow-safe checked arithmetic added in
0.32.0; only the newly-supported label references need the post-layout
resolution step.

## Validation

All 40 Windows tests passed, including an executable end-to-end check: an
exported function's object was linked and executed, and a REL32 reference
from a backend-generated object was resolved against it. Additional tests
specifically covered output-path handling — directories containing spaces,
a missing `-o` argument, and unwritable destinations — because `-o` is new
CLI surface area that raw-mode output never had to validate.

Source-level `extern` declarations and separate data sections remain future
frontend work; 0.33.0 supports exactly one instruction-only `.text` section
and one exported label per object.

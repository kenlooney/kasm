# Genesis: A Lexer for Two Days (Sept 12)

Kasm began on **September 12** with a bare CMake C project: an executable
target, an install rule, and a CI workflow before there was anything
interesting to build. The very first working piece of the assembler was not
the assembler at all — it was the **lexer**.

## Building the front door first

The earliest commits are almost entirely about turning raw source text into
tokens:

- Identifier and integer-literal tokens, plus a shared "digit value" helper
  that would later grow into hex, octal, and binary literal support.
- Newline tokens, because whitespace-sensitivity mattered even before there
  was a parser to consume it.
- Single-line (`//`) and multi-line (`/* */`) comment handling.
- Small, isolated test drivers (`tests/lexer_driver.c` and CMake script
  fixtures) that checked token streams directly, long before there was any
  encoded output to check instead.

This is a pattern that holds for the entire project: **a capability doesn't
exist until it has a permanent test**. The lexer tests for decimal/binary
literals and comment handling are still part of the test suite today.

## Source ownership from day one

Even at this early stage, the project made a decision that shows up
throughout its history: the assembler *owns* its source text rather than
borrowing a pointer into the caller's buffer. `source.c`/`source.h` were
split out of `main.c` almost immediately (`c7de71a — Refactor project
structure: move main.c to src, add source.c and source.h`), establishing the
module boundaries — `source`, `lexer`, `program`, `semantic`, `layout`,
`encode`, `decode` — that the codebase still uses.

## Housekeeping that paid off later

A few unglamorous early commits mattered more than they looked:

- CI workflow and versioned-tag release process, set up before there was a
  release worth making (`4f86da7`, `8008415`, `f26c88f`).
- License headers added to `main.c` for compliance.
- GCC build instructions added to the README so the project would build on
  more than one toolchain from the start — this is what let later 16-bit and
  overflow-safety work be validated on both Windows and WSL2/GCC.

By the end of these first two days, the project had no assembler behavior a
user would recognize — but it had a lexer, a test harness, a CI pipeline, and
a release process. Everything from 0.1.0 onward was built on that
foundation.

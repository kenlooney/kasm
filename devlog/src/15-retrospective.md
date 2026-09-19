# Retrospective: Six Days, Thirty-Four Versions

Every version tag from 0.1.0 to 0.33.0 was created between **September 12
and September 18, 2026** — six calendar days for thirty-three tagged
versions (0.13.0 skipped). Version 0.34.0 followed on the same day as a
documentation-only milestone: the launch of this journal. That pace is only
sustainable because of habits that were established early and never
abandoned:

## What held up across the whole project

- **One capability per version.** Nearly every version in this journal adds
  exactly one instruction, one directive, or one hardening fix — rarely
  more. That granularity is what makes a 6-day, 34-version history legible
  at all instead of a blur.
- **A permanent regression test before moving on.** From the very first
  lexer commits through COFF object output, no capability shipped without a
  test that stayed in the suite. `tests/encode_file.cmake`'s "assemble, then
  diff hex and/or decoded listing" pattern, established around 0.1.0–0.3.0,
  is still how most instruction-level features are checked today.
- **Two toolchains in CI from day one.** Windows/MSVC and WSL2/GCC both
  building and testing every milestone caught real bugs — a missing
  `<string.h>` include at 0.26.0, and a NUL-unsafe test at 0.25.0/0.25.1 —
  that a single-toolchain project would likely have shipped.
- **A layout pass, introduced early (0.8.0), that every forward-reference
  feature since has depended on** — jumps, relocation, TIMES/FILL counts,
  `$`/`$$` expressions, and label-valued data all lean on the same offset
  assignment pass rather than reinventing it.
- **Explicit rejection over silent misbehavior.** `--bits` mode rejection
  (0.22.0), the `[bx]`-only memory operand (0.27.0), rejecting `jmp abs` in
  COFF output (0.33.0) — the project consistently chose to say "not
  supported yet" clearly rather than let an unsupported case fall through to
  incorrect output.

## What's still explicitly future work

As of 0.33.0, the README and this journal agree on what hasn't been built
yet:

- General 16-bit addressing modes beyond `[bx]`.
- A defined boot-sector entry convention (segment state, stack, BIOS
  services) — the 0.28.0 fixture is byte-correct but not yet bootable.
- Source-level `extern` declarations and multiple data sections for COFF
  objects.
- Converting `jmp abs` into a linker relocation for object output.
- Full unsigned 32-bit/64-bit expression values for `dd`/`dq` (the storage
  width already exceeds what the expression evaluator currently allows).
- Consistent cleanup of `parser.nodes`, `program.statements`, and encoded
  byte buffers on error paths (a short-lived concern for a CLI process, but
  worth fixing before Kasm is used as anything but a one-shot tool).

## Why this book exists

None of the above is a criticism — it's the normal shape of an assembler
built version-by-version with test-gated milestones. This journal exists so
that shape stays visible: the order features arrived in, the constraints
that shaped each one, and the handful of fix-up versions (0.18.1, 0.24.1,
0.25.1) that are just as instructive as the feature versions around them.

Future entries in this journal should keep the same discipline: one chapter
per meaningful version or tightly related group of versions, written close
to when the work happens, sourced from the actual commits and tests rather
than from memory after the fact.

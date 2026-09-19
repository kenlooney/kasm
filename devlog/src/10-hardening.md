# 0.19.1–0.22.0 — Hardening the Frontend

Not every version added an instruction. A few were dedicated entirely to
paying down assumptions baked into earlier, faster milestones.

## 0.19.1 — Source stops being a fixed 4096-byte buffer

`c06e86c — Bump version to 0.19.1 and implement dynamic source buffer
allocation` replaced the original fixed-size, 4096-ASCII-byte source buffer
(`include/source.h`) with dynamically growing storage. The NUL-free ASCII
validation rule was kept — only the size ceiling was removed. This mattered
because every later feature that grows source size (data directives with
long lists, TIMES/FILL, boot-sector padding fixtures) would otherwise have
run straight into an artificial 4 KB ceiling that had nothing to do with the
instruction set.

## 0.21.0 — Straightening out opcode handling

`6809007 — enhance opcode handling for MOV, ADD, and SUB instructions`
refactored how register codes fed into the `0x81`-family ModR/M byte
selection, in preparation for adding `edx` support (`1384ac9`) without
duplicating the `eax`/`ecx` special-casing that had accumulated.

## 0.22.0 — CPU mode stops being implicit

`4036930 — Update project version to 0.22.0; implement target mode handling
and reject unsupported modes` added `--bits 16`, `--bits 32`, and `--bits 64`
to the CLI, and made 16-bit and 32-bit modes explicitly *rejected* rather
than silently mishandled. This is a defensive design choice: rather than
letting an unsupported mode fall through to encoding logic that only ever
assumed 64-bit registers, the CLI now fails fast with a clear mode error.
That rejection was later narrowed as 16-bit support actually arrived (a
regression test now checks that `eax` specifically is rejected in 16-bit
mode, rather than rejecting the whole mode outright).

This sequence — remove an artificial limit, straighten out internal opcode
handling, then make previously-implicit assumptions explicit and checked —
is a recognizable pattern any time Kasm was about to expand into genuinely
new territory. It shows up again just before the 16-bit work begins in
earnest, and again just before COFF output.

# 0.23.0–0.25.1 — Data Directives and Location-Aware Layout

Up to 0.22.0, every statement in a Kasm program was an instruction. This era
added a second category entirely: **data that isn't an instruction at all**.

## 0.23.0 — DB/DW, then DD/DQ

`4af6afc` added `db`/`byte` and `dw`/`word` directives; two follow-up commits
(`ca576ae`, `63bc4b3`) added `dd`/`dword` and `dq`/`qword`. All four accept
comma-separated expression lists terminated by `;`, emit little-endian
values, and are range-checked against their declared width:

| Spelling | Bytes | Range |
| --- | ---: | --- |
| `db`/`byte` | 1 | 0–255 |
| `dw`/`word` | 2 | 0–65535 |
| `dd`/`dword` | 4 | 0–2147483647 (current expression limit) |
| `dq`/`qword` | 8 | 0–2147483647 (current expression limit) |

The README is explicit that DD and DQ's *storage* is wider than the
*expression evaluator's* current range — the evaluator rejects results above
2147483647 before a value ever reaches the wider storage, so the diagnostic
range and the storage width intentionally don't match yet.

Because data isn't an instruction, the CLI had to stop assuming every image
was disassemblable: images containing data print `Data emitted; instruction-
only decoding skipped.` instead of attempting a listing.

`45ab217 — Refactor instruction size calculation for ST_SUB_RIM and
ST_ADD_RIM` landed alongside this work, fixing layout's instruction-size
accounting to account for register codes correctly — a reminder that adding
a second statement category (data) forced a re-check of assumptions the
layout pass had made when everything was an instruction.

## 0.24.0 / 0.24.1 — Repetition, then a compatibility fix

`4c160d6 — Implement TIMES/FILL support for data directives` added
`times <count>` / `fill <count>` prefixes that repeat a directive's entire
value list, with overflow-safe `size_t` count validation. `642c3d3 — Bump
version to 0.24.1` immediately followed to restore the default
single-source-file CLI invocation to 64-bit mode — the explicit `--bits`
flag from 0.22.0 had apparently changed default behavior in a way that broke
existing tests and installed-package smoke checks, and 0.24.1 exists purely
to fix that compatibility regression.

## 0.25.0 / 0.25.1 — `$` and `$$`, and a CI portability fix

`7ff337f — Bump version to 0.25.0; add location-aware expressions` let
`$` (current image offset) and `$$` (image section start) appear inside
TIMES/FILL counts, resolved during layout after preceding statement sizes are
known. The canonical example is NASM-style boot-sector padding:

```asm
mov eax, 42;
times 510-($-$$) db 0;
dw 0xAA55;
```

`28d1e4e — Bump version to 0.25.1` fixed the regression test for this
exact example: the original 512-byte padding check compared binary output
directly, but CMake strings can't safely carry embedded NUL bytes, which
made the test's behavior differ between Windows and Linux CI. The fix
measures length through the CLI's **hex** output instead, which is
NUL-safe and portable — a small but instructive lesson about testing binary
output through text-based tooling.

Data directives, once added, immediately needed most of the same
infrastructure instructions had — layout, range checks, tests — which is why
this era reads as dense as the arithmetic-ISA growth in the previous
chapter, just for a different statement kind.

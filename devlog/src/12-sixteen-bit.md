# 0.26.0–0.30.0 — The 16-Bit Adventures

Five versions turned `--bits 16` from a rejected mode (0.22.0) into a mode
with a real, if intentionally narrow, instruction subset.

## 0.26.0 — MOV/ADD/SUB on AX, CX, DX

`d3a6cac — Bump version to 0.26.0; add initial 16-bit MOV/ADD/SUB support`
introduced operand-width as a property tracked on each statement, so layout
and encoding could agree on instruction length without guessing from the
target mode alone:

| Form | Size | Bytes |
| --- | ---: | --- |
| `mov ax, 42;` | 3 | `B8 2A 00` |
| `mov cx, 7;` | 3 | `B9 07 00` |
| `add ax, 7;` | 3 | `05 07 00` |
| `sub cx, 3;` | 4 | `81 E9 03 00` |

Validation here was unusually thorough for a "0.x-in-development" milestone:
all 32 registered tests passed under WSL2/GCC in addition to Windows, and the
GCC build caught a missing `<string.h>` include that MSVC had silently
tolerated — a concrete payoff for keeping a second toolchain in CI since the
project's earliest days.

## 0.27.0 — One memory operand, deliberately narrow

`0fcd4a2 — Bump version to 0.27.0; add 16-bit indirect MOV support from
[bx] to AX/CX/DX` added exactly one addressing form: `mov ax, [bx];` →
`8B 07`. The README is explicit that this is intentionally limited to
`[bx]` — no other memory operand is supported. Scoping a feature to the
single simplest case, with a name that makes the limitation obvious, is a
recurring way Kasm avoids implying more capability than it has.

## 0.28.0 — A boot-sector-shaped fixture

`19b5d52 — Bump project version to 0.28.0` combined 16-bit instructions,
location-aware TIMES padding, and binary literals into a 512-byte image
fixture ending in the `55 AA` boot signature. The README is careful to call
this a **layout fixture**, not a bootable program — there's no defined entry
convention, segment state, stack, or BIOS-service usage yet. It looks like a
boot sector and is exactly the right size, but running it in a real (or
emulated) BIOS boot path is future work.

## 0.29.0 — Segment registers

`d314108 — Update project version to 0.29.0; add support for segment-
register push/pop` added `push`/`pop` for `es`, `cs`, `ss`, `ds`, `fs`, and
`gs`. The legacy four use single-byte opcodes; `fs`/`gs` need the `0F`
two-byte escape. `pop cs` is deliberately not encoded — there is no valid
modern x86 form for it, so the omission is a correctness decision, not a gap.

## 0.30.0 — Subtract with borrow

`sbb` joined the 16-bit immediate forms for `ax`/`cx`/`dx`, using the same
`81 /3` ModR/M family as the earlier `sub` support. The README notes plainly
that `sbb` *consumes* the processor carry flag as part of its semantics, but
Kasm neither executes code nor otherwise establishes that flag — the
encoding honors the contract; nothing in Kasm currently fulfills it.

Across this whole arc, the 16-bit instruction set stayed deliberately
narrow — a handful of registers, one memory form, no general addressing
modes — while still being real enough to produce a byte-exact, test-covered
boot-sector-shaped image. That restraint is a design choice as much as
anything encoded in opcodes.

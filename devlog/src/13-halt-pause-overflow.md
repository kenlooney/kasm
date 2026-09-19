# 0.31.0–0.32.0 — Odds, Ends, and Overflow Safety

Two short versions closed out gaps that had been quietly accumulating:
missing trivial instructions, and unchecked signed-integer overflow in
constant folding.

## 0.31.0 — HALT and PAUSE

`cc2b435 — Bump version to 0.31.0; add operand-free HALT and PAUSE
instructions with tests` added the simplest possible instruction shape:
opcodes with no operands at all.

| Instruction | Bytes |
| --- | --- |
| `halt` | `F4` |
| `pause` | `F3 90` |

The interesting detail is `pause`: it's a two-byte sequence (`F3 90`), and
the decoder has to recognize it as a whole before falling through to
treating `90` as a bare `nop`-shaped byte it doesn't otherwise understand.
Getting the *order* of decoder checks right — multi-byte sequences before
single-byte fallbacks — is a small but real source of bugs in any
byte-pattern decoder, and this version's test checks both the binary and the
decoded listing together to guard against it.

## 0.32.0 — Constant folding stops trusting signed overflow

`e5805ad — Bump version to 0.32.0; implement overflow-safe arithmetic for
constant expressions with tests` replaced unchecked signed `long long`
arithmetic in constant folding with checked addition, subtraction, and
multiplication, ahead of Kasm's existing signed-32-bit language limit. This
closed a real correctness gap flagged during project review: signed integer
overflow is undefined behavior in C, and the previous folding logic could
overflow *before* the 32-bit range check ever ran, meaning the check itself
couldn't be trusted for large enough inputs. The fix checks for overflow at
each individual operation, not just at the end result, and the regression
test explicitly exercises overflowing addition, subtraction, and
multiplication, alongside the existing precedence test.

Neither of these versions is glamorous, but they're the kind of maintenance
that has to happen between "add a new instruction" milestones — closing a
decoder ordering gap and an undefined-behavior gap before building the next
major feature (COFF object output) on top of a semantic layer that folds
expressions correctly.

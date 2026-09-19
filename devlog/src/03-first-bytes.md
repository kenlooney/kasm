# 0.1.0 — The First Encoded Bytes

Version **0.1.0** is the first release-worthy milestone: Kasm can parse and
validate `mov` programs, evaluate their expressions, and print the encoded
bytes.

## Semantic checking arrives before encoding

`381a87b — Implement semantic checking and evaluation` added the step
between parsing and encoding that still exists today: folding the expression
AST into concrete integer values and validating that a `mov`'s destination
register and immediate value make sense. This is the origin of the
`check_program` stage in the pipeline documented in the project's engineering
notes (`source_load → parse_program → check_program → layout → encode`).

Putting semantic validation *before* encoding — rather than validating during
encoding — set a pattern the project kept for every instruction added after
it: new instructions get a parser rule, a semantic check, and an encoder
entry, in that order, each backed by its own test.

## `mov eax, expr;`

`f0e5274 — Implement encoding for MOV instructions` is the payoff commit: it
takes a validated statement and emits real x86-64 bytes (`B8 imm32`) for
`mov eax, <expression>;`. The accompanying tests compared CLI output against
expected hex for the first time, establishing the "encode, then diff the hex"
testing style used throughout the rest of the project (see
`tests/encode_file.cmake`).

At this point Kasm could turn `mov eax, 40+2;` into five bytes of machine
code and print them. It could not yet write those bytes anywhere useful, run
them, or return from them — that arrives over the next three versions.

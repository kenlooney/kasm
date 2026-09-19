# Parsing Expressions and Programs

With tokens in hand, the next stretch of commits built a **recursive-descent
expression parser** and then wrapped it in a **program** structure that could
hold more than one statement.

## Expressions, in the order operator precedence demands

The parser grew in the textbook order for precedence climbing:

1. Integer literals and parenthesized error handling (`b3ea2aa`).
2. Addition and subtraction (`c0daf03`).
3. Multiplication and parenthesized precedence (`99d11f7`) — multiplication
   had to bind tighter than the addition/subtraction level added just before
   it, and parentheses had to override both.

Each step landed with its own parser test, so by the time semantic evaluation
existed, the precedence rules were already locked down by regression tests
rather than by hope.

## From a single expression to a program

`6be02df — Implement program parsing and testing` introduced the core model
that the rest of the assembler still uses: a **`Program`** is a flat,
growable array of `Statement` values (`include/program.h`). Two tests still
in the suite trace directly back to this commit:

- `program_blocks.c` / `parser_blocks.cmake` — multiple statements parse into
  the same flat array.
- `program_growth.c` — the array actually grows past its initial capacity
  without corrupting existing entries.

## Nested blocks are parser sugar, not a runtime concept

`08cdd02 — Add support for nested blocks in parser` added `{ }` grouping with
an explicit depth limit. Nested blocks never became a distinct concept in the
`Program` model — they are flattened into the same statement array during
parsing. This is a deliberate simplification: nothing downstream (layout,
encoding, decoding) ever needs to know a block existed. It is worth calling
out specifically because it is easy to assume, from the syntax alone, that
blocks are scopes or basic blocks in a compiler sense. They are not; they are
purely a source-level grouping convenience.

By the end of this phase, Kasm could parse a full program of expressions and
statements — but it still couldn't validate or encode anything. That came
next, in 0.1.0.

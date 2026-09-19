# 0.2.0–0.4.0 — RET, Binary Output, and Generated Headers

Three fast, small versions turned "prints hex" into "produces something you
could plausibly link into a program."

## 0.2.0 — `ret;`

`0cdc732 — Implement RET instruction support` added the `C3` opcode. Trivial
in isolation, but it meant a Kasm program could now be a *complete, callable
function body* rather than just a `mov`: `mov eax, 40+2; ret;` is the
canonical example that reappears throughout the README and this journal.

## 0.3.0 — Bytes that leave the process

`3b83695 — Update project version to 0.3.0, add binary writing functionality,
and implement reusable example encoding tests` introduced `write_binary`,
saving the encoded image to `program.bin`. This is also when the project
adopted its per-example, isolated-output-directory testing convention: each
example `.asm` file gets its own test output directory so parallel test runs
never collide over `program.bin`.

## 0.4.0 — Bytes a C compiler can embed

`6327c4a — Update project version to 0.4.0, implement write_c function, and
add generated output files` added `write_c`, emitting `generated.h` as a C
byte array plus its length. A tiny inspector program could now `#include` the
generated header and print the bytes back out — the first sign that Kasm's
output was meant to be consumed by *other* programs, not just inspected as
hex on a terminal.

By 0.4.0, the round trip was: assemble → validate → encode → save as
`.bin` → save as a C header. The only thing missing was actually running the
result, which is the subject of the next chapter.

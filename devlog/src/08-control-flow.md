# 0.14.0 — Conditional Branches and a Decoder

Version 0.14.0 (0.13.0 was skipped) did two things that changed how the
project validated itself from this point forward.

## INC/DEC and JZ/JNZ

`e0269d1 — Update project version to 0.14.0, add INC and DEC instructions,
and implement conditional jumps (JZ, JNZ)` gave Kasm its first real loop
primitive: increment or decrement `eax`, then branch on whether the result
was zero. Combined with the near/short jumps from 0.10.0, this was enough to
express a counted loop for the first time.

## A decoder, and a new kind of test

`1ec928a — Implement decoding functionality and update tests for new
instructions` added a **decoder** — the mirror image of the encoder, turning
an image back into a textual listing. This changed the testing strategy: up
to this point, tests compared encoded hex against expected hex. From here on,
tests could also compare a *decoded listing* against expected text, which
catches a different class of bug (an encoder and decoder can independently
agree on the wrong thing, but round-tripping through both makes an encoding
error much more likely to surface as a decoding error too).

This decoder is also the origin of a bug that took several versions to
surface. Because new instructions were added to the encoder and their
decoder entries sometimes lagged behind, `inc eax` and `jz` briefly had
correct encodings but incomplete decoding support — a gap the 0.18.1
milestone (see the next chapter) closed with a dedicated regression test.

With INC/DEC and JZ/JNZ in place, and a decoder to check work against, the
project moved on to broadening the arithmetic instruction set: ADD, SUB, and
several bitwise operations, all following the same "parse → validate →
encode → decode" pattern established here.

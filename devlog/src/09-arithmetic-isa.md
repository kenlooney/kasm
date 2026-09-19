# 0.15.0–0.20.0 — Growing the Arithmetic ISA

This stretch of versions is the most repetitive-looking part of Kasm's
history, and that repetition is the point: once the pattern of "parser rule →
semantic check → encoder entry → decoder entry → permanent test" was
established, adding an instruction became a mechanical, low-risk process.

| Version | Instruction(s) added | Encoded form |
| --- | --- | --- |
| 0.15.0 | `add`/`sub eax, imm32` | `05 imm32` / `2D imm32` |
| 0.16.0 | `or eax, imm32`, `push`/`pop rax` | `0D imm32` |
| 0.17.0 | `adc eax, imm32` | `15 imm32` |
| 0.18.0 | `int imm8` | unsigned 8-bit vector, range-checked |
| 0.18.1 | *(fix)* decoding for `inc eax` and `jz` | no new encoding, decoder-only fix |
| 0.19.0 | `cmp`/`and eax, imm32`, `jb`/`jl` | `3D imm32` / `25 imm32` |
| 0.19.1 | *(hardening)* dynamic source buffer | no new instruction |
| 0.20.0 | `xor eax, imm32` | `35 imm32` |

## The 0.18.1 regression fix is worth reading closely

`103547d — Update project version to 0.18.1, add INC and JZ instruction
support, and include regression tests for decoding` is a bug-fix release
inserted into the middle of otherwise linear feature growth. It closed the
decoder gap noted in the previous chapter: `inc eax` (`FF C0`) and `jz`
(`0F 84`) had valid encoders but an incomplete decoder, so a valid program
would encode correctly, write its outputs, and then have the CLI exit with
`unknown or truncated encoding` while printing the decoded listing. The fix
added the missing decoder cases and a permanent end-to-end regression
covering both instructions' byte output *and* their decoded listing
together, exactly the kind of test the 0.14.0 decoder made possible.

## Registers beyond `eax`

Growing from `eax`-only forms to `ecx` and `edx` forms (folded into this era
via `6809007` and `1384ac9`) introduced the "second byte of the `0x81`
family selects the operation and register" pattern documented in the
project's opcode lookup guide — `81 C1` is `add ecx, imm32`, `81 E9` is
`sub ecx, imm32`. This is the first time Kasm had to disambiguate more than
one instruction sharing an opcode prefix, which is also why register codes
started being tracked explicitly on statements rather than inferred at
encode time.

By 0.20.0, Kasm had a genuinely useful arithmetic and bitwise instruction
set for `eax`. What it didn't have yet was any explicit acknowledgment that
16-bit and 32-bit targets even existed as distinct concepts — that
correction came next.

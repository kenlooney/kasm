# Semantic checking and encoding

## Semantic checking and evaluation

The [semantic checker](https://github.com/kenlooney/kasm/blob/main/src/semantic.c)
evaluates expression nodes in dependency order, then validates operands and
stores each MOV, ADD, SUB, OR, AND, ADC, CMP, or INT immediate in
`Statement.value`. For example:

```asm
mov eax, (10+4)*3;
```

Produces:

```text
B8 2A 00 00 00
```

MOV supports EAX in the established 32-bit form and EDI for MOV-immediate in
64-bit mode; PUSH/POP require `rax`. Every literal and intermediate expression result must fit the signed
32-bit range `-2147483648..2147483647`. The final MOV immediate must also be
nonnegative, giving an accepted range of `0..2147483647`. These are the
current language restrictions. The lexer's larger literal range does not
bypass semantic checking.

Examples rejected by the semantic checker:

| Input | Diagnostic |
| --- | --- |
| `mov ebx, 7;` | `only register eax is supported` |
| `mov eax, 1-2;` | `mov immediate must be nonnegative in this language` |
| `mov eax, 2147483647+1;` | `expression outside signed 32-bit range` |

Intermediate results are checked too: `mov eax, (2147483647+1)-1;` is
rejected even though its final mathematical result would fit. Evaluation
computes values for later encoding; it does not execute instructions or
modify CPU registers.

## Encoding and current limitations

The encoder emits five bytes for 32-bit MOV: opcode `B8` for EAX or `BF`
for EDI, followed by the evaluated immediate as four bytes in little-endian order. For example, 42
becomes `2A 00 00 00`. RET emits one byte, `C3`. The byte buffer grows
dynamically as instructions are appended.

The current language supports MOV, ADD, SUB, OR, AND, ADC, CMP, INC, and
DEC on `eax`, PUSH/POP on `rax`, INT with an immediate vector, operand-free
RET, 64-bit SYSCALL, short, near, or absolute indirect JMP, and near JZ/JNZ/JB/JL to a
label. Far jumps, short conditional jumps, other condition codes, other
instructions and registers, labels in expressions, memory operands,
directives beyond DB/DW/DD/DQ and their aliases, and object or executable
profiles beyond COFF and the minimal Linux ELF64 writer are not implemented. MOV immediates must be in
`0..2147483647`; ADD/SUB/OR/ADC accept signed 32-bit expression results,
subject to the expression restrictions below. INT requires a final value in
`0..255`. Blocks provide grouping, not scope or control flow. Empty input or
empty blocks print an empty hex line and `Decoding successful.`, write an
empty binary, and generate a header with `code_size = 0` and a placeholder
array element so the declaration remains valid C.

### Freestanding Linux entry instructions

In 64-bit mode, `mov edi, <expression>;` emits `BF` followed by a four-byte
little-endian immediate. `syscall;` emits `0F 05`, occupies two bytes, and
takes no operand. Both forms are rejected in 16-bit mode. Together with EAX
MOV they express the payload in
[`examples/exit42.asm`](https://github.com/kenlooney/kasm/blob/main/examples/exit42.asm).
The payload uses Linux syscall number 60 and status 42; Kasm does not yet wrap
it in an ELF executable.

### ADD and SUB immediate expressions

`add eax, <expression>;` adds the evaluated immediate to EAX at runtime;
`sub eax, <expression>;` subtracts it. Only lowercase `eax` is accepted. The
parser requires a comma and a final semicolon, just as for MOV.

| Instruction | Opcode | Immediate | Total size |
| --- | --- | --- | --- |
| `add eax,7;` | `05` | `07 00 00 00` | 5 bytes |
| `sub eax,7;` | `2D` | `07 00 00 00` | 5 bytes |

The destination EAX is implicit in these opcodes. The immediate is four
bytes in little-endian order. Layout reserves five bytes for either
instruction, and the decoder prints the immediate as a signed value.

ADD/SUB accept expression results in `-2147483648..2147483647`; MOV retains
its nonnegative restriction. Every literal and intermediate result must
still fit the existing signed 32-bit expression limits. Unary minus can be
written as `-1` or `-(1+1)`. Runtime arithmetic wraps to 32 bits and updates
arithmetic flags, including carry; it does not use the assembler's
expression-overflow checks.

[examples/add.asm](https://github.com/kenlooney/kasm/blob/main/examples/add.asm)
loads 35 and adds `3+4`, returning 42:

```asm
mov eax,35;
add eax,3+4;
ret;
```

Its bytes are `B8 23 00 00 00 05 07 00 00 00 C3`. The permanent
`encode.add` test checks these bytes and the listing in
[tests/add_expected.txt](https://github.com/kenlooney/kasm/blob/main/tests/add_expected.txt).
Run it after building:

```powershell
ctest --test-dir build/windows-debug -C Debug -R encode.add --output-on-failure
```

The SUB counterpart `mov eax,49; sub eax,3+4; ret;` produces
`B8 31 00 00 00 2D 07 00 00 00 C3`. Both examples returned 42 in manual
WSL2 execution checks. SUB's bytes and listing were also checked manually;
SUB does not yet have a permanent CTest entry. These runtime checks did not
measure flags.

### Bitwise OR

`or eax, <expression>;` combines the current EAX value with the evaluated
immediate, setting each result bit if that bit is set in either operand. It
requires `eax`, a comma, an expression, and a semicolon. The expression uses
the same signed 32-bit limits as ADD/SUB; a negative result supplies its
32-bit two's-complement bit pattern. This adds a runtime instruction, not a
new expression operator.

The encoding is `0D` followed by four immediate bytes in little-endian
order, for a total of five bytes. The decoder reads the immediate and
prints `or eax, <value>` with a signed decimal value.

```asm
mov eax,40;
or eax,2;
ret;
```

The program produces `B8 28 00 00 00 0D 02 00 00 00 C3`. The binary
patterns `00101000` (40) and `00000010` (2) combine to `00101010` (42).
Exact bytes and the decoded listing were checked manually, and WSL2
execution returned `result = 42`. These checks are not yet registered as a
permanent CTest test, and flags were not directly tested.

### Bitwise AND

`and eax, <expression>;` combines the current EAX value with the evaluated
immediate, clearing each result bit unless it is set in both operands. It
requires `eax`, a comma, an expression, and a semicolon. The expression uses
the same signed 32-bit limits as ADD/SUB/OR; a negative result supplies its
32-bit two's-complement bit pattern.

The encoding is `25` followed by four immediate bytes in little-endian
order, for a total of five bytes. The decoder reads the immediate and
prints `and eax, <value>` with a signed decimal value.

```asm
mov eax,42;
and eax,-1;
ret;
```

This produces `B8 2A 00 00 00 25 FF FF FF FF C3`. Unary negative
expressions are supported, so `-1` is equivalent to `0-1`.

### Adding with carry

`adc eax, <expression>;` adds the evaluated immediate and the current carry
flag (CF) to EAX at runtime:

```text
EAX = EAX + immediate + CF
```

It requires lowercase `eax` and accepts signed 32-bit expression results
under the same expression limits as ADD/SUB. Its encoding is `15` followed
by four immediate bytes in little-endian order, for five bytes total. For
example, `adc eax,2+3;` emits `15 05 00 00 00` and decodes as `adc eax, 5`.

The CPU performs 32-bit arithmetic and updates arithmetic flags, including
CF. ADC consumes the incoming carry and produces a new carry, which lets
additions propagate carry between parts of a larger number. MOV does not
change CF, so loading EAX alone does not establish a known carry value.

This example explicitly sets carry before ADC:

```asm
mov eax,0;
sub eax,1;
mov eax,10;
adc eax,2+3;
ret;
```

Subtracting one from zero sets CF; the following MOV preserves it. ADC
therefore computes `10 + 5 + 1` and returns 16. Change the first
instruction to `mov eax,1;` and SUB clears CF, so the program returns 15
instead.

Both versions passed exact-byte and decoded-listing checks, and WSL2
execution returned 16 and 15 respectively. The existing 15 Windows CTest
tests also passed. The ADC checks are currently temporary manual checks
under `build/adc-check`, not permanent CTest entries. They verify incoming
carry behavior, but do not directly measure the outgoing flags.

### Software interrupt encoding

`int <expression>;` takes one immediate operand, with no register or
comma. The internal statement kind is `ST_INT_IMM8`. The expression is
evaluated during assembly and must produce a value in `0..255`; values
outside that range report `interrupt vector must be in range 0..255`.
Existing expression limits still apply to literals and intermediate
results.

The encoding is opcode `CD` followed by a single unsigned vector byte.
Layout reserves two bytes, and the decoder displays the vector in
hexadecimal:

| Source | Bytes | Decoded instruction |
| --- | --- | --- |
| `int 0;` | `CD 00` | `int 0x00` |
| `int 0x10;` | `CD 10` | `int 0x10` |
| `int 8+8;` | `CD 10` | `int 0x10` |
| `int 255;` | `CD FF` | `int 0xFF` |

To check encoding, save `int 0x10;` as `examples/interrupt.asm` and run
from the repository root after rebuilding:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/interrupt.asm" "-DEXPECTED_HEX=CD 10" -P tests/encode_file.cmake
```

This test assembles and decodes the bytes; it does not execute the
interrupt. INT support alone does not add a real-mode target, boot-image
generation, or BIOS services to the existing x86-64 Windows/WSL2 runner.
BIOS-style use of `int 0x10` needs the appropriate execution environment
and CPU mode.

Manual tests checked the four valid cases above, each followed by RET to
verify the next decoded offset is 2. Negative (`0-1`) and oversized (`256`)
vectors were rejected. All 15 existing CTest tests passed. The temporary
INT checks live under `build/int-check`; they are not permanent CTest
entries, and no interrupts were executed.

### Saving and restoring RAX

`push rax;` emits `50`, and `pop rax;` emits `58`. Both occupy one
instruction byte, but in the x86-64 runner they transfer an eight-byte
register value to or from the stack. Their source operand must be `rax`,
not `eax`; other operands report `push/pop require register rax`. They
have no immediate expression.

A balanced sequence can save a value while another instruction changes EAX:

```asm
mov eax,42;
push rax;
mov eax,99;
pop rax;
ret;
```

The expected result is 42. Restore the stack before `ret` so it reads the
caller's return address. The assembler does not verify stack balance.

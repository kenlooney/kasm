# The instruction set and parser

The [statement parser](https://github.com/kenlooney/kasm/blob/main/src/program.c)
accepts `mov <identifier>, <expression>;`, `add <identifier>, <expression>;`,
`sub <identifier>, <expression>;`, `or <identifier>, <expression>;`,
`xor <identifier>, <expression>;`, `and <identifier>, <expression>;`,
`adc <identifier>, <expression>;`, `cmp <identifier>, <expression>;`,
`int <expression>;`, `push <identifier>;`, `pop <identifier>;`, `halt;`,
`pause;`, the operand-free instruction `ret;`, `jmp near <identifier>;`,
`jmp short <identifier>;`, `jmp abs <identifier>;`, `inc <identifier>;`,
`dec <identifier>;`, `jz <identifier>;`, `jnz <identifier>;`,
`jb <identifier>;`, and `jl <identifier>;`, as well as `identifier:` label
definitions. For example, save this as `example.asm`:

```asm
mov eax, 40+2;
mov eax, 0b00000111;
mov eax, (2+3)*4;
```

Run it after building with one of the presets below:

```powershell
.\build\windows-debug\Debug\kasm.exe example.asm
```

```bash
./build/GCC-debug/kasm example.asm
```

Both print:

```text
B8 2A 00 00 00 B8 07 00 00 00 B8 14 00 00 00
```

Comments can appear on their own lines or alongside instructions:

```asm
// Set up the first value.
mov eax, 40+2; // A single-line comment runs to the end of this line.

/* This comment spans multiple lines.
   Use it to explain a group of instructions. */
mov eax, 0b00000111;

mov eax, (2+3) /* Multiply the grouped sum by four. */ *4;
```

This example prints the same encoded bytes shown above. The semicolons
terminate the instructions; `//` and `/* ... */` introduce comments. Block
comments do not nest.

Instruction names are case-sensitive: use lowercase `mov`, `ret`, `jmp`,
`inc`, `dec`, `jz`, `jnz`, `jb`, `jl`, `add`, `sub`, `or`, `xor`, `and`,
`adc`, `cmp`, `push`, `pop`, and `int`. The parser accepts an identifier as
the destination; semantic validation then requires lowercase `eax` for
arithmetic and MOV, or `rax` for PUSH/POP. Each instruction requires a
semicolon, including the last one. Statements can share a line or be
separated by newlines and blank lines; a trailing newline is optional. Line
breaks within an instruction are not supported, except inside block comments.

## Label definitions

[examples/labels.asm](https://github.com/kenlooney/kasm/blob/main/examples/labels.asm)
demonstrates multiple labels on one line:

```asm
entry: alias: mov eax,42; done: ret;
```

Each label is an identifier followed by a colon, with no semicolon. Labels
can appear before instructions, on their own lines, consecutively, or inside
blocks. The parser stores each definition as an `ST_LABEL` statement, and the
encoder emits no bytes for it. This sample therefore produces the same six
bytes as `mov eax,42; ret;`:

```text
B8 2A 00 00 00 C3
```

Labels receive byte offsets during the layout pass described below.
Duplicate names are rejected. `jmp` accepts a label name as its operand,
including one defined later in the source. Labels are not supported as
expression values.

Run the reusable test from the repository root after rebuilding:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/labels.asm" "-DEXPECTED_HEX=B8 2A 00 00 00 C3" -P tests/encode_file.cmake
```

## Instruction offsets and label lookup

The [layout pass](https://github.com/kenlooney/kasm/blob/main/src/layout.c)
runs after semantic checking and before byte encoding. Starting at byte
offset zero, it stores the current offset in each `Statement.offset`, then
advances by `instruction_size()`: five bytes for MOV, ADD, SUB, OR, ADC, CMP,
or a near JMP, two for a short JMP, fourteen for an absolute JMP including
its address slot, two for INC, DEC, or INT, six for JZ or JNZ, one for RET,
PUSH, or POP, and zero for a label. Offsets are relative to the beginning of
the encoded program, not source-file positions or runtime memory addresses.

[examples/label_offsets.asm](https://github.com/kenlooney/kasm/blob/main/examples/label_offsets.asm)
contains:

```asm
entry: mov eax,42; done: ret;
```

Its layout is:

| Statement | Byte offset | Encoded size |
| --- | --- | --- |
| `entry:` | 0 | 0 |
| `mov eax,42;` | 0 | 5 |
| `done:` | 5 | 0 |
| `ret;` | 5 | 1 |

The total size is six bytes. Labels do not advance the offset, so
consecutive labels name the same location. Blocks do not introduce a
separate label scope. Label names are case-sensitive; repeating a name
anywhere in the program reports `duplicate label` during layout.

After layout, `label_offset()` from
[include/layout.h](https://github.com/kenlooney/kasm/blob/main/include/layout.h)
looks up a label token and returns its byte offset through an output
parameter. It returns 1 when found, or reports `undefined label` and returns
0 when absent. For this example, `entry` resolves to 0 and `done` to 5. The
encoder uses this lookup API to resolve jump targets after all statements
have received offsets.

To check this sample's encoded output after rebuilding:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/label_offsets.asm" "-DEXPECTED_HEX=B8 2A 00 00 00 C3" -P tests/encode_file.cmake
```

This reusable test verifies the emitted bytes, not the stored offsets or
lookup results. The CLI does not print offsets; inspecting `Statement.offset`
or calling `label_offset()` in a C test checks those directly.

## Near jumps and forward references

`jmp near <label>;` encodes an unconditional near jump as `E9` followed by a
signed 32-bit displacement in little-endian order. Every near jump occupies
five bytes; the assembler does not automatically choose a shorter encoding.

The displacement is relative to the end of the jump instruction:

```text
displacement = target offset - (jump offset + 5)
```

Layout assigns offsets to the entire program before encoding, so a target
may be defined before or after the jump. A backward jump has a negative
displacement. An absent target reports `undefined label`; a displacement
outside the signed 32-bit range reports `near jump outside signed 32-bit
range`.

[examples/near_jump.asm](https://github.com/kenlooney/kasm/blob/main/examples/near_jump.asm)
demonstrates a forward reference:

```asm
jmp near answer; mov eax,99; answer: mov eax,42; ret;
```

| Statement | Byte offset | Encoded size |
| --- | --- | --- |
| `jmp near answer;` | 0 | 5 |
| `mov eax,99;` | 5 | 5 |
| `answer:` | 10 | 0 |
| `mov eax,42;` | 10 | 5 |
| `ret;` | 15 | 1 |

The displacement is `10 - (0 + 5) = 5`. The full 16-byte encoding is:

```text
E9 05 00 00 00 B8 63 00 00 00 B8 2A 00 00 00 C3
```

The jump skips the MOV that sets `eax` to 99 and lands on the MOV that sets
it to 42. Test the expected bytes from the repository root after rebuilding:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/near_jump.asm" "-DEXPECTED_HEX=E9 05 00 00 00 B8 63 00 00 00 B8 2A 00 00 00 C3" -P tests/encode_file.cmake
```

## Short jumps and explicit jump syntax

Jump instructions require a lowercase `near`, `short`, or `abs` modifier
followed by a label name and a semicolon. Omitting the modifier reports
`expected near, short, or abs after jmp`. The internal statement kinds are
`ST_NEAR_JMP`, `ST_SHORT_JMP`, and `ST_ABS_JMP`.

| Source syntax | Opcode | Displacement | Total size |
| --- | --- | --- | --- |
| `jmp near label;` | `E9` | Signed 32-bit, little-endian | 5 bytes |
| `jmp short label;` | `EB` | Signed 8-bit, -128 through 127 | 2 bytes |

The `abs` form uses an indirect jump and an address slot, described below.

A short jump uses `target offset - (jump offset + 2)`. The assembler checks
the range and reports `short jump outside signed 8-bit range` if the target
is too far away. It does not automatically widen a short jump to a near
jump.

[examples/short_jump.asm](https://github.com/kenlooney/kasm/blob/main/examples/short_jump.asm)
contains:

```asm
jmp short answer; mov eax,99; answer: mov eax,42; ret;
```

Here `answer` is at offset 7 and the jump ends at offset 2, so the
displacement is `+5`. The complete output is 13 bytes, three fewer than the
near-jump version:

```text
EB 05 B8 63 00 00 00 B8 2A 00 00 00 C3
```

Run the byte check from the repository root after rebuilding:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/short_jump.asm" "-DEXPECTED_HEX=EB 05 B8 63 00 00 00 B8 2A 00 00 00 C3" -P tests/encode_file.cmake
```

## Absolute indirect jumps

`jmp abs <label>;` emits a six-byte RIP-relative indirect jump followed by
an eight-byte address slot. Layout reserves all fourteen bytes:

```text
FF 25 00 00 00 00 | eight-byte target slot
```

The four zero displacement bytes make the instruction read its destination
from the slot immediately after it. The slot initially contains the target
label's image offset in little-endian order. The encoder records a
relocation at `statement offset + 6`; the loader replaces the stored offset
with the loaded image's base address plus that offset before execution. The
CPU jumps to the address it reads; it does not execute the slot as
instructions.

This uses a full 64-bit destination rather than a signed relative
displacement. It is an indirect near jump in x86 terminology, not a
segment-changing far jump. The current source operand must still name a
label in the same image.

For example:

```asm
jmp abs answer; mov eax,99; answer: mov eax,42; ret;
```

`answer` is at offset 19 (`0x13`). The 25-byte image before relocation is:

```text
FF 25 00 00 00 00 13 00 00 00 00 00 00 00 B8 63 00 00 00 B8 2A 00 00 00 C3
```

The generated header has `patch_count = 1` and `patch_offsets[] = {6}`. The
encoder allows up to 256 patches and reports `too many relocation patches`
before exceeding that capacity. An unknown target reports `undefined label`.

To try this example, save it as `examples/abs_jump.asm`, rebuild Kasm, and
run from the repository root:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/abs_jump.asm" "-DEXPECTED_HEX=FF 25 00 00 00 00 13 00 00 00 00 00 00 00 B8 63 00 00 00 B8 2A 00 00 00 C3" -P tests/encode_file.cmake
```

For execution, follow the runner commands in
[Generated C data, execution, and decoding](execution-and-decoding.md),
substituting `abs_jump.asm` for `program.asm` when generating the header
beside the runner. Relocation is required before executing this image. The
example's exact bytes and patch metadata were checked, and the relocated
image executed in WSL2 with `result = 42`.

## Changing EAX and branching on flags

`inc eax;` adds one to EAX at runtime, and `dec eax;` subtracts one. Both
require lowercase `eax`; other registers report `only register eax is
supported`. Unlike `mov eax,43-1;`, which evaluates its expression during
assembly, `mov eax,43; dec eax;` performs the subtraction when the
generated code runs.

| Instruction | Encoding | Size |
| --- | --- | --- |
| `inc eax;` | `FF C0` | 2 bytes |
| `dec eax;` | `FF C8` | 2 bytes |

These instructions perform 32-bit arithmetic with wraparound: decrementing
zero produces `FFFFFFFF`, and incrementing that value produces zero. They
update arithmetic flags, including ZF (set when the result is zero), while
preserving the carry flag. Assembly-time expression range checks still apply
to MOV expressions; they do not limit runtime arithmetic.

`jz <label>;` jumps when ZF is 1, and `jnz <label>;` jumps when ZF is 0.
`jb <label>;` jumps when CF is 1, and `jl <label>;` jumps when SF differs
from OF. Otherwise execution continues with the next instruction. These
instructions read the existing flags; they do not themselves test EAX or
change the flags. Unlike `jmp`, they take a label directly, with no `near`,
`short`, or `abs` modifier.

| Instruction | Opcode | Displacement | Total size |
| --- | --- | --- | --- |
| `jz label;` | `0F 84` | Signed 32-bit, little-endian | 6 bytes |
| `jnz label;` | `0F 85` | Signed 32-bit, little-endian | 6 bytes |
| `jb label;` | `0F 82` | Signed 32-bit, little-endian | 6 bytes |
| `jl label;` | `0F 8C` | Signed 32-bit, little-endian | 6 bytes |

All four resolve labels after layout and calculate
`target offset - (instruction offset + 6)`. Missing targets report
`undefined label`; out-of-range displacements report `conditional jump
outside signed 32-bit range`.

For example, this loop counts down from three and returns zero:

```asm
mov eax,3;
again:
dec eax;
jnz again;
ret;
```

The label is at offset 5 and JNZ ends at offset 13, so the displacement is
-8:

```text
B8 03 00 00 00 FF C8 0F 85 F8 FF FF FF C3
```

To check it with the reusable script, save the snippet as
`examples/countdown.asm` and run from the repository root after rebuilding:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/countdown.asm" "-DEXPECTED_HEX=B8 03 00 00 00 FF C8 0F 85 F8 FF FF FF C3" -P tests/encode_file.cmake
```

Manual byte and decode checks covered INC, DEC, forward conditional
branches, the backward loop, and forward and backward JB/JL targets. WSL2
execution verified 43 decrementing to 42, 41 incrementing to 42, zero
returning to zero after DEC then INC, both taken and untaken paths for JZ
and JNZ, and the countdown returning zero. Invalid register operands were
also rejected. The branch tests exercise ZF behavior; JB/JL runtime flags
and other flags, including carry preservation, were not directly measured.

## Comparing EAX and setting flags

`cmp eax, <expression>;` compares EAX with a signed 32-bit immediate without
changing EAX. It updates the arithmetic flags as if the immediate were
subtracted from EAX, including ZF, which allows a following conditional
jump to test the result. The operand must be lowercase `eax`; other
registers report `only register eax is supported`.

| Instruction | Encoding | Immediate | Size |
| --- | --- | --- | --- |
| `cmp eax, expression;` | `3D` | Signed 32-bit, little-endian | 5 bytes |

The expression is evaluated during assembly and must be in the signed
32-bit range. For example:

```asm
mov eax,42;
cmp eax,40+2;
ret;
```

This produces the following bytes and leaves EAX unchanged at runtime:

```text
B8 2A 00 00 00 3D 2A 00 00 00 C3
```

The decoded listing includes `cmp eax, 42`.

## Blocks

Braces group statements into blocks, which can contain other blocks:

```asm
mov eax, 1;
{
    // Blank lines and comments are allowed inside blocks.
    mov eax, 40+2;

    {
        mov eax, (2+3)*4;
    }
}
{ mov eax, 0b00000111; }
{} // Empty blocks are valid too.
```

This prints:

```text
B8 01 00 00 00 B8 2A 00 00 00 B8 14 00 00 00 B8 07 00 00 00
```

Statements retain their source order in one flat statement array; blocks do
not create scopes or separate AST nodes. Each MOV still needs its
semicolon, but a closing brace takes no semicolon.

Blocks may nest up to `MAX_BLOCK_DEPTH`, currently 16 in
[include/program.h](https://github.com/kenlooney/kasm/blob/main/include/program.h).
This guard bounds recursive parser calls to protect the C call stack. A 17th
nested block reports `blocks nested too deeply`. Missing and extra closing
braces report `expected closing brace` and `unexpected closing brace`,
respectively.

The [expression parser](https://github.com/kenlooney/kasm/blob/main/src/expr.c)
supports integer literals, unary `-`, binary `+` and `-`, multiplication
(`*`), and parentheses. Multiplication binds more tightly than addition and
subtraction; operators at the same precedence associate left to right. Thus
`2+3*4` parses as `2+(3*4)`, while `(2+3)*4` groups the addition first.
Parentheses may nest up to 32 levels. Unary `+`, symbols, division, and
other expression operators are not supported yet.

Each MOV, ADD, SUB, OR, AND, ADC, CMP, or INT statement references its
expression's root in the parser's node array. Statement storage grows
dynamically, starting at 16 entries and doubling as needed; there is no
fixed 256-statement limit. Source files are stored in a dynamically growing
buffer.

The CLI exits with status 0 after successful encoding and file output, and
1 on a loading, lexing, parsing, semantic, allocation, or file-output error,
or incorrect command-line usage. Diagnostics go to stderr.

## Opcode lookup guide

This quick reference is useful when you are extending the encoder and
decoder for another register form. The general pattern is: parse the
register, store a small numeric register code, then switch on that code
during encoding and match the byte patterns during decoding.

| Source form | Encoded bytes | Meaning |
| --- | --- | --- |
| `mov eax, imm32;` | `B8 imm32` | Move a 32-bit immediate into `eax`. |
| `mov ecx, imm32;` | `B9 imm32` | Move a 32-bit immediate into `ecx`. |
| `add eax, imm32;` | `05 imm32` | Add a signed 32-bit immediate to `eax`. |
| `add ecx, imm32;` | `81 C1 imm32` | Add a signed 32-bit immediate to `ecx`. |
| `sub eax, imm32;` | `2D imm32` | Subtract a signed 32-bit immediate from `eax`. |
| `sub ecx, imm32;` | `81 E9 imm32` | Subtract a signed 32-bit immediate from `ecx`. |
| `or eax, imm32;` | `0D imm32` | Bitwise OR with `eax`. |
| `xor eax, imm32;` | `35 imm32` | Bitwise XOR with `eax`. |
| `and eax, imm32;` | `25 imm32` | Bitwise AND with `eax`. |
| `cmp eax, imm32;` | `3D imm32` | Compare `eax` against the immediate. |
| `adc eax, imm32;` | `15 imm32` | Add with carry into `eax`. |

The second byte in the `0x81` family decides the exact operation and
register:

- `81 C1` = `add ecx, imm32`
- `81 E9` = `sub ecx, imm32`

This is the same idea as the decoder: match the emitted bytes in reverse,
then print back the matching source instruction. In other words, the
decoder is the mirror image of the encoder for these register-aware
instruction forms.

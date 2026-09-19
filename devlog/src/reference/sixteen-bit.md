# The 16-bit instruction set

Use `--bits 16` for the MOV, ADD, and SUB forms on AX, CX, and DX described
below. The default remains `--bits 64`, using EAX, ECX, and EDX for these
operations. 32-bit target mode is still unsupported. Operand width is
stored on each statement so layout and emission agree on instruction
length.

## MOV, ADD, and SUB on AX/CX/DX

| 16-bit form | Size | Example bytes |
| --- | ---: | --- |
| `mov ax,42;` | 3 | `B8 2A 00` |
| `mov cx,7;` | 3 | `B9 07 00` |
| `mov dx,3;` | 3 | `BA 03 00` |
| `add ax,7;` | 3 | `05 07 00` |
| `sub cx,3;` | 4 | `81 E9 03 00` |
| `add dx,7;` | 4 | `81 C2 07 00` |

The decoder receives the target mode: the same MOV opcode has a two-byte
immediate in these 16-bit forms and a four-byte immediate in the
supported 64-bit forms. ADD/SUB register forms also include a ModR/M byte
where needed.

Run the permanent MOV fixture after configuring and building:

```powershell
ctest --test-dir build/windows-debug -C Debug -R "^encode.mode16_mov$" --output-on-failure
```

Run the arithmetic example with the reusable file test from the
repository root:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/mode16_add_sub.asm" "-DBITS=16" "-DEXPECTED_HEX=05 07 00 81 E9 03 00 81 C2 07 00" -P tests/encode_file.cmake
```

For WSL2, use `-DKASM=build/GCC-debug/kasm` and the same source, mode, and
expected bytes. Outputs go into the test's isolated directory under
`build/example-tests`. Omitting `BITS` from the helper still selects
64-bit mode.

These are encoding/decoding checks, not execution tests. The existing
hosted runners execute x86-64 code and must not be used to execute these
16-bit images. This does not establish general 16-bit instruction support,
complete operand-range validation, or a bootable-program workflow; other
instructions still need mode-specific review.

Validation: all 32 registered tests passed in WSL2 with GCC, and the
16-bit arithmetic example matched its expected bytes in both Windows and
WSL2. The seven targeted Windows decoder/encoding regressions also
passed. The obsolete blanket 16-bit rejection test now checks rejection
of EAX in 16-bit mode. The GCC build also caught and prompted a missing
`<string.h>` fix.

## Indirect MOV from `[bx]`

16-bit mode supports loading AX, CX, or DX from the `[bx]` memory operand.
For example, `mov ax, [bx];` emits `8B 07` and is decoded back to the same
instruction. This feature is intentionally limited to `[bx]`; other
memory addressing forms remain unsupported.

Run the focused regression test from the repository root:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/mode16_mov_indirect.asm" "-DBITS=16" "-DEXPECTED_HEX=8B 07" -P tests/encode_file.cmake
```

## Boot-sector image fixture

The 16-bit boot-sector example combines instruction encoding,
location-aware padding, and binary literal parsing:

```asm
mov ax, 42;
add ax, 3;
times (512-0b00000010)-($-$$) db 0;
dw 0xAA55;
```

The resulting image is designed to be exactly 512 bytes, with zero-filled
padding and the boot signature `55 AA` in its final two bytes. The image
begins with the two 16-bit instructions and is suitable for loading at
`0x7C00` in a firmware emulator or raw-image virtual machine. It is a
tested layout fixture, not yet a bootable sector — see
[Data directives and layout expressions](data-directives.md) for what
"not yet bootable" specifically excludes.

## Segment-register push/pop

16-bit mode supports pushing and popping the segment registers covered by
the current instruction subset. The legacy registers use one-byte
opcodes, while FS and GS use two-byte `0F` opcode sequences:

| Instruction | Bytes |
| --- | --- |
| `push es` / `pop es` | `06` / `07` |
| `push cs` | `0E` |
| `push ss` / `pop ss` | `16` / `17` |
| `push ds` / `pop ds` | `1E` / `1F` |
| `push fs` / `pop fs` | `0F A0` / `0F A1` |
| `push gs` / `pop gs` | `0F A8` / `0F A9` |

`pop cs` is not encoded because it has no valid modern x86 instruction
form. The exact-byte segment-register regression is run with:

```powershell
ctest --test-dir build/windows-debug -C Debug -R "^encode.mode16_segment$" --output-on-failure
```

## Subtract-with-borrow

16-bit mode supports immediate `sbb` for AX, CX, and DX. The instruction
uses the `81 /3` opcode family, so the register code is stored in the low
three bits of the ModR/M byte:

| Instruction | Bytes |
| --- | --- |
| `sbb ax, 1` | `81 D8 01 00` |
| `sbb cx, 2` | `81 D9 02 00` |
| `sbb dx, 3` | `81 DA 03 00` |

The immediate is little-endian and the decoder prints signed 16-bit
values. The exact-byte and exact-listing regression is run with:

```powershell
ctest --test-dir build/windows-debug -C Debug -R "^encode.mode16_sbb$" --output-on-failure
```

`sbb` consumes the processor carry flag; the assembler encodes that
contract but does not execute or otherwise establish the flag.

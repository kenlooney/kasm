# Data directives and layout expressions

Data directives emit values directly, without an instruction opcode:

| Spelling | Bytes per value | Accepted result |
| --- | ---: | --- |
| `db` or `byte` | 1 | 0 through 255 |
| `dw` or `word` | 2 | 0 through 65535 |
| `dd` or `dword` | 4 | 0 through 2147483647 (current expression limit) |
| `dq` or `qword` | 8 | 0 through 2147483647 (current expression limit) |

All four accept comma-separated expressions terminated by a semicolon.
Multi-byte values use little-endian byte order, with no automatic
alignment or padding.

```asm
db 60+5,66,0;       // 41 42 00
byte 0xAA,0x55;    // AA 55
dw 4660;           // 34 12
word 0x55AA;       // AA 55
dd 0x12345678;     // 78 56 34 12
dword 40+2;        // 2A 00 00 00
dq 42;            // 2A 00 00 00 00 00 00 00
qword 0;          // 00 00 00 00 00 00 00 00
```

Each word list element occupies two bytes: `word 0xAA,0x55;` emits
`AA 00 55 00`. Empty lists, trailing commas, missing commas, negative
results, and results above the directive's range are rejected. The
existing signed 32-bit expression rules still apply to intermediate
calculations. DD and DQ store four and eight bytes respectively, but full
unsigned 32-bit and 64-bit expression values are not yet supported. Even
though their current range diagnostics name wider storage limits, the
expression evaluator rejects results above 2147483647 first.

Prefix a data directive with `times <count>` or `fill <count>` to repeat
its complete comma-separated list. The count is an expression, must be
nonnegative, and must fit `size_t`.

```asm
times 3 db 170;          // AA AA AA
fill 2 dw 4660,0;        // 34 12 00 00 34 12 00 00
times 2 dd 42;           // 2A 00 00 00 2A 00 00 00
```

Labels include the size of preceding data. Executable examples must jump
over embedded data, as in
[examples/data_all_jump.asm](https://github.com/kenlooney/kasm/blob/main/examples/data_all_jump.asm),
which mixes all four widths. Data-only examples are encoding fixtures, not
functions to execute.

The CLI writes source-named `.bin` and `.h` files for images containing
data, but prints `Data emitted; instruction-only decoding skipped.`
instead of trying to disassemble the image. Instruction-only images retain
their decoded listing. TIMES/FILL repetition is supported for data
directives. Strings, alignment, and symbol-valued data remain future work.

Generated images are capped at **16 MiB (16,777,216 bytes)** by
`KASM_IMAGE_LIMIT`. This output limit includes instructions and repeated
data; it is separate from dynamically allocated source-file storage.

## Location-aware padding

The assembler also supports `$` and `$$` inside repeat-count expressions:

```asm
mov eax, 42;
times 510-($-$$) db 0;
dw 0xAA55;
```

`$` is the current image offset and `$$` is the image start, which is
offset zero for the current single-image layout. The count is resolved
during layout, after earlier statements have known sizes. In this
example, `mov eax, 42;` is five bytes, so the padding count is
`510 - (5 - 0) = 505`, followed by the little-endian signature bytes
`55 AA`.

This produces a 512-byte data image. It is a tested layout fixture, not
yet a bootable sector: a signature alone does not define an entry
convention, segment state, stack, BIOS services, or an emulator workflow.
Negative padding is rejected before the repeat count is converted to
`size_t`.

Permanent tests cover all eight spellings, expression values, range
boundaries, exact bytes, mixed-width layout, jump targets, constant and
location-aware TIMES/FILL repetition, and invalid lists and ranges. The
Windows Debug build validates the data-directive suite, including
`encode.times_fill`, `encode.boot_pad`, and invalid-input cases within
`semantic.data_invalid`. After configuring and building, run them with:

```powershell
ctest --test-dir build/windows-debug -C Debug -R "(encode.data_|encode.times_fill|encode.boot_pad|semantic.data_invalid)" --output-on-failure
```

Symbol-valued data (label expressions inside directives) resolves after
layout as of 0.33.0; constant-only expressions retain the checked
arithmetic described in
[Control instructions and overflow safety](control-and-overflow.md).

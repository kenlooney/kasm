# Generated C data, execution, and decoding

## Generated C data and execution

The [C writer](https://github.com/kenlooney/kasm/blob/main/src/output.c)
wraps the encoded bytes in `program.h`, including an include guard and
`<stddef.h>`. For
[examples/program.asm](https://github.com/kenlooney/kasm/blob/main/examples/program.asm),
the declarations contain these values (shown compactly):

```c
static const unsigned char code[] = {
    0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3,
};
static const size_t code_size = 6;
static const size_t patch_offsets[] = {0};
static const size_t patch_count = 0;
```

The examples below require a source checkout and a C compiler. After
building Kasm with the `GCC-debug` preset, run these commands from the
repository root on Linux x86-64 or x86-64 WSL2:

```bash
cd examples/generated
../../build/GCC-debug/kasm ../program.asm
cc -std=c11 inspect.c -o inspect
./inspect
```

[inspect.c](https://github.com/kenlooney/kasm/blob/main/examples/generated/inspect.c)
includes the generated header and prints its contents without executing
them:

```text
Size: 6
B8 2A 00 00 00 C3
```

To execute the same example:

```bash
cc -std=c11 -I../../include runner.c -o runner
./runner
```

Expected output:

```text
result = 42
```

[runner.c](https://github.com/kenlooney/kasm/blob/main/examples/generated/runner.c)
allocates writable memory with `mmap`, copies the array into it, applies
relocation patches, changes the memory to readable and executable with
`mprotect`, calls it as an `int` function with no arguments, and releases
the memory with `munmap`. The example sets the return value in `eax` and
returns with `ret`.

### Windows x86-64

Open a **Visual Studio Developer Command Prompt**. Initialize the x64
tools before compiling; an x86 compiler produces a 32-bit runner even on
64-bit Windows. Starting from the repository root after building Kasm:

```bat
call "%VSINSTALLDIR%VC\Auxiliary\Build\vcvarsall.bat" x64
cd examples\generated
..\..\build\windows-debug\Debug\kasm.exe ..\program.asm
cl /W4 /std:c11 inspect.c /Fe:inspect.exe
inspect.exe
cl /W4 /std:c11 /I..\..\include runner.c /Fe:runner.exe
runner.exe
```

The compiler banner should say **for x64**. The inspector prints the same
six bytes as on Linux, and the runner prints `result = 42`.

The Windows runner uses `VirtualAlloc` to allocate writable memory, copies
and relocates the bytes, then uses `VirtualProtect` to make it readable
and executable, `FlushInstructionCache` before calling the function, and
`VirtualFree` to release the allocation.

Both runner implementations target x86-64 and use a platform-specific
function-pointer conversion. They reject empty programs and non-x86-64
processes. Use a complete function such as `examples/program.asm`, whose
final instruction is `ret;`. After changing the assembly, regenerate the
header and recompile the C examples so they use the new bytes.

### Load-time relocation

The helper in
[include/relocate.h](https://github.com/kenlooney/kasm/blob/main/include/relocate.h)
converts an image-relative offset into an absolute address after the
loader knows where the image resides:

```text
patched address = load address + stored image offset
```

Each entry in `patch_offsets` identifies the start of an eight-byte field
in the loaded image. `relocate()` reads that field as a little-endian
unsigned 64-bit offset, adds the image's base address, and writes the
resulting address back into the same field. The patch location and the
target offset are distinct: a patch at offset 0 containing the value 8
becomes `base + 8` stored at offset 0.

The helper rejects patches whose eight-byte fields extend outside the
image and targets at or beyond the image size. A target outside the image
reports `relocation target outside code`. With zero patches, it leaves the
image unchanged. Apply relocation once to a freshly copied image while the
memory is writable, before changing its protection and executing it.

Generated headers now include `patch_offsets` and `patch_count` alongside
`code` and `code_size`. A zero-count patch array contains a placeholder
zero; that placeholder is not applied. `program.bin` contains only image
bytes, without the patch table. Current relative jumps need no relocation
because their source and target move together when the image is loaded.

The encoder adds one relocation entry for each `jmp abs` address slot.
Programs using only MOV, RET, and relative jumps still generate
`patch_count = 0`. A small demonstration in `main.c` patches a separate
16-byte buffer; it does not add a relocation to the assembled program. The
absolute-jump example in
[The instruction set and parser](instruction-set.md) exercises relocation
of actual generated code.

For the runner workflow, regenerate `program.h` from an existing example
such as `program.asm` using the commands above, then recompile the runner.
The include path option is required to find `relocate.h`. Running the
reusable encoding test writes its header under `build/example-tests`, not
beside the example runner.

Manual checks on Windows x64 and WSL2 verified the patched value equals
the buffer address plus 8, rejection of invalid patch bounds and an
out-of-range target, and unchanged data for zero patches. Both runners
compiled and returned 42 with an ordinary program containing no
relocation entries. In 0.12.0, the WSL2 runner also executed the
absolute-jump example with one actual relocation and returned 42. These
manual checks supplement the CTest suite; they are not currently
registered as CTest tests.

## Decoding the generated bytes

The [decoder](https://github.com/kenlooney/kasm/blob/main/src/decode.c)
inspects the encoded byte buffer after the CLI has written its output
files. It displays instruction offsets and reconstructed operands; it does
not execute the program. Offsets and targets are decimal, with instruction
offsets padded to at least four digits.

[examples/decode.asm](https://github.com/kenlooney/kasm/blob/main/examples/decode.asm)
contains:

```asm
mov eax,3; loop: dec eax; jnz loop; mov eax,42; ret;
```

The complete CLI output is:

```text
B8 03 00 00 00 FF C8 0F 85 F8 FF FF FF B8 2A 00 00 00 C3
0000: mov eax, 3
0005: dec eax
0007: jnz target=5
0013: mov eax, 42
0018: ret
Decoding successful.
```

The JNZ displacement is -8: adding it to the instruction's end at offset
13 recovers target offset 5. Original label names, comments, and
expression spelling cannot be recovered from the bytes.

The current decoder recognizes MOV, ADD, SUB, OR, ADC, and CMP EAX
immediate, INT imm8, PUSH/POP RAX, RET, DEC/INC EAX, short and near JMP,
near JZ/JNZ/JB/JL, and Kasm's fourteen-byte absolute-jump convention. For
the latter, it reads the unrelocated address slot as an image offset and
skips all fourteen bytes. Its display still uses `jmpabs
image-offset=...`; relative JMP displays `jmp target=...`. This is
inspection output, not source in Kasm's explicit `jmp abs`, `jmp near`, or
`jmp short` syntax.

Unknown or truncated encodings print `unknown or truncated encoding` and
cause the CLI to exit with status 1, even though the output files have
already been written. This decoder is limited to its supported patterns;
it is neither a general x86 disassembler nor a verifier that code is safe
to execute.

The `decode.example` CTest checks both the exact 19-byte image and the
entire listing, including the backward target and success message.
Expected text is stored in
[tests/decode_expected.txt](https://github.com/kenlooney/kasm/blob/main/tests/decode_expected.txt).
Run it after building:

```powershell
ctest --test-dir build/windows-debug -C Debug -R decode.example --output-on-failure
```

The test writes its artifacts to
`build/windows-debug/examples/Debug/decode`, separately from other
examples. To reuse the script directly:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/decode.asm" "-DEXPECTED_HEX=B8 03 00 00 00 FF C8 0F 85 F8 FF FF FF B8 2A 00 00 00 C3" "-DEXPECTED_DECODE_FILE=tests/decode_expected.txt" -P tests/encode_file.cmake
```

# Extending the assembler: a checklist

Use this order when adding an instruction. ADD and SUB are useful examples
of the full path from source text to bytes and back.

1. **Choose the exact syntax and encoding.** Match the complete operand
   form and CPU mode in the instruction reference, not just the mnemonic.
   Write down the opcode, any prefixes or ModR/M bytes, immediate width,
   total size, and effects on registers and flags. Calculate a small
   expected byte sequence by hand before implementing it. `add eax,7;`
   uses `05 07 00 00 00`; `sub eax,7;` uses `2D 07 00 00 00`. Each
   instruction occupies five bytes.

2. **Add a statement kind** in
   [include/program.h](https://github.com/kenlooney/kasm/blob/main/include/program.h).
   Give it a consistent name, such as `ST_ADD_RIM` or `ST_SUB_RIM`.
   Existing fields are sufficient for these forms: `operand` stores the
   register token, `expression` stores the expression root, `value`
   stores its evaluated immediate, and `offset` stores the instruction's
   position in the image.

3. **Parse the operands** in
   [src/program.c](https://github.com/kenlooney/kasm/blob/main/src/program.c).
   Recognize the mnemonic and assign its statement kind. For
   register/immediate instructions, follow MOV's pattern: register,
   comma, expression, semicolon. Save the register token in `s.operand`
   and the result of `parse_expression()` in `s.expression`. INC/DEC's
   one-operand parser is not enough for ADD/SUB. Ordinary instruction
   names are already identifier tokens; change the lexer only if the new
   syntax introduces something it cannot tokenize.

4. **Validate and evaluate** in
   [src/semantic.c](https://github.com/kenlooney/kasm/blob/main/src/semantic.c).
   Include the new kind in the register check and, when it has an
   immediate expression, in the code that assigns `s->value` from the
   evaluated expression. Decide the accepted range explicitly. ADD/SUB
   currently use signed 32-bit expression results; MOV additionally
   requires a nonnegative immediate. Forgetting the value assignment can
   silently encode zero instead of the requested value.

5. **Reserve the full size** in
   [src/layout.c](https://github.com/kenlooney/kasm/blob/main/src/layout.c).
   Add the kind to `instruction_size()`. Count every emitted byte,
   including prefixes, operands, and embedded address slots. ADD/SUB
   reserve five bytes. An incorrect size shifts later labels and breaks
   jumps even if the instruction's own bytes look correct.

6. **Emit bytes** in
   [src/encode.c](https://github.com/kenlooney/kasm/blob/main/src/encode.c).
   Select the correct opcode and write the operand in its required
   format. `little_endian(bytes, value, 4)` writes four operand bytes,
   not a four-byte instruction. ADD/SUB each write one opcode byte
   followed by four immediate bytes. `FF` is an opcode group, not a
   prefix to put before every instruction. For branches, resolve the
   target and calculate a displacement from the instruction's end; for
   address slots, record relocation patches instead. Propagate
   allocation/write failures.

7. **Recognize the bytes** in
   [src/decode.c](https://github.com/kenlooney/kasm/blob/main/src/decode.c).
   Check enough bytes remain before reading operands, print the
   instruction and its reconstructed operands, and advance by the
   complete encoded size. Match the chosen signed or unsigned
   interpretation of immediates. The CLI currently decodes after writing
   its files, so forgetting this step can make a correctly encoded
   program exit with `unknown or truncated encoding`.

8. **Add an example and a permanent test.** Put a small source file under
   `examples/` and register it in
   [CMakeLists.txt](https://github.com/kenlooney/kasm/blob/main/CMakeLists.txt).
   Use `add_example_test(name examples/name.asm "EXPECTED HEX")` for byte
   checks. Follow `encode.add` when also checking the listing: pass
   `EXPECTED_DECODE_FILE` to the reusable script and commit that text
   fixture under `tests/`. Hand-calculated expectations should be
   independent of the encoder. Files under `build/` are temporary and do
   not become regression tests automatically.

9. **Rebuild, test, and inspect runtime behavior.** Run the build and
   CTest commands in [Building and testing](building-and-testing.md).
   Cover a normal value, zero, supported negative expressions, range
   boundaries, invalid registers, and malformed operands as appropriate.
   Test layout with a label or jump after the new instruction. For
   execution, regenerate `program.h` beside the runner, then recompile
   the runner before running it. Check the result against a value
   calculated by hand; test flags explicitly when their behavior
   matters. Run Windows and WSL2 checks before a release.

10. **Update documentation and the development milestone.** Record the
    syntax, supported operands, byte format, limits, and test coverage in
    this reference, and add a chapter to the dev journal describing why
    the feature was added. Check the version in CMake and the version
    history agree. A new feature does not require an immediate release.

For example, `mov eax,49; sub eax,3+4; ret;` follows this path: the
parser stores the expression, semantic checking computes 7, layout
reserves five bytes for SUB, the encoder emits `2D 07 00 00 00`, the
decoder prints `sub eax, 7`, and execution returns 42. The expression is
evaluated during assembly; the subtraction from EAX happens at runtime.

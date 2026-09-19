# Building and testing

## Reusable example encoding test

[tests/encode_file.cmake](https://github.com/kenlooney/kasm/blob/main/tests/encode_file.cmake)
assembles an existing file and checks that the source-named `.bin` bytes
match the first hexadecimal line of stdout. Pass `EXPECTED_HEX` to also
check the expected instruction encoding. Run from the repository root
after building:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/program.asm" "-DEXPECTED_HEX=B8 2A 00 00 00 C3" -P tests/encode_file.cmake
```

Change `SOURCE` to reuse the script with another example; omit
`EXPECTED_HEX` when you only want to check successful assembly and
binary/stdout consistency. By default, each source path gets its own
directory under `build/example-tests`. The script prints the resulting
binary path. Repeating the same example replaces its previous binary. You
can override the directory with `-DOUTPUT_DIR=<path>`; use different
directories for examples that run concurrently.

CTest uses this script for `examples/mov_42.asm`, `examples/ret.asm`, and
`examples/program.asm`, saving separate binaries under
`build/windows-debug/examples/Debug/<test-name>/<input-name>.bin` with the
Windows Debug preset. To register another example, add a call inside
`BUILD_TESTING`:

```cmake
add_example_test(my_example examples/my_example.asm "B8 07 00 00 00 C3")
```

## Configure and run the suite

Build from a source checkout with CMake and a C compiler. The Windows
presets target Visual Studio 2026 with the C++ build tools installed. The
Linux/WSL2 preset uses GCC and Make. Use a CMake version that supports
your generator and the repository's version-8 preset file; the basic
CMake project requires 3.20 or newer when configuring without presets.

The registered CTest tests cover:

- Exact MOV encoding: `mov eax,42;` produces `B8 2A 00 00 00`.
- ADD expression evaluation, exact emitted bytes, and the decoded listing
  from `examples/add.asm`.
- Exact decoding of `examples/decode.asm`, including instruction offsets,
  the backward JNZ target, and the success message.
- Exact RET and combined MOV/RET encoding, with saved binary bytes
  checked against stdout and expected bytes in separate example
  directories.
- Semantic evaluation of `mov eax, (10+4)*3;` to `42`.
- Exact expression ASTs for integers, addition/subtraction,
  multiplication precedence, and parentheses overriding precedence.
- Multiple MOV statements with LF and CRLF line endings, binary literals
  in instructions, evaluated values (including inside nested blocks), and
  a missing-semicolon diagnostic.
- Dynamic storage growth to 300 MOV statements, preserving their
  operands.
- Empty, nested, and sibling blocks; blank lines within blocks; statement
  order and expression references; missing/extra brace diagnostics; and
  acceptance at `MAX_BLOCK_DEPTH` with rejection one level beyond it.
- Integer literal lexing, comments, newline tokens, LF/CRLF/CR line
  endings, unterminated-comment diagnostics, and long sequences of
  adjacent comments.
- The 16-bit, data-directive, control-instruction, and overflow-safety
  suites described in their own reference chapters.
- ELF header and program-header fields, exact entry payload, invalid entry and
  unsupported-input rejection, plus exit status 42 when tests run on Linux.

Expression tests use `expr_test_driver` to inspect ASTs independently of
the encoding CLI. Semantic and multiple-statement tests use
`semantic_test_driver` to inspect statement counts and evaluated values.
Lexer tests use `lexer_test_driver`. These drivers are built only when
testing is enabled. The label and jump samples, C inspection, and
Linux/Windows execution examples in
[Generated C data, execution, and decoding](execution-and-decoding.md)
are manual checks; they are not currently registered as CTest tests.

Run these commands from the repository root (the directory containing
`CMakePresets.json`). Configure and build before running CTest:

```powershell
cmake --preset windows-debug -DBUILD_TESTING=ON
cmake --build --preset windows-debug
ctest --test-dir build/windows-debug -C Debug --output-on-failure -V
```

```bash
cmake --preset GCC-debug -DBUILD_TESTING=ON
cmake --build --preset GCC-debug
ctest --test-dir build/GCC-debug -C Debug --output-on-failure -V
```

The presets create separate build directories under `build`; CTest must
point to the configured directory. For Release, use `windows-release` in
all three commands and replace `-C Debug` with `-C Release`.

If your terminal is already in `build/GCC-debug`, run `cd ../..` first to
return to the repository root before using the commands above.

The commands include `-V` for verbose output even when tests pass; omit
it for a shorter report.

After adding a new source file, rerun the configure command before
building so the generated project includes it.

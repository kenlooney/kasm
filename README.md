# Kasm — Ken's Assembler

Kasm currently loads a source file, parses MOV statements, nested blocks, and
expression ASTs, validates operands, and evaluates expressions. It encodes
`mov eax, <expression>;` as x86 machine-code bytes and prints them as hexadecimal
text. Version 0.1.0 is an early assembler implementation with a deliberately
small instruction set; it does not generate object files or executables.

## Using a release

Download the Windows x64 or Linux x64 ZIP from
[GitHub Releases](https://github.com/kenlooney/kasm/releases) and extract it.
Each archive contains `bin/kasm.exe` (Windows) or `bin/kasm` (Linux), plus this
README and the Apache 2.0 license under `share/doc/kasm`.

From the extracted package directory, create `example.asm` containing
`mov eax,42;`, then run:

```powershell
.\bin\kasm.exe example.asm
```

```bash
./bin/kasm example.asm
```

Expected output:

```text
B8 2A 00 00 00
```

The CLI accepts exactly one source-file path. Output goes to stdout as one line
of uppercase hexadecimal bytes, with a space after each byte and a final
newline. Redirecting stdout saves hex text, not a raw binary file. Instructions
are encoded in source order and are not executed. Windows packages use the
static MSVC runtime; Linux packages are built on Ubuntu 24.04.

Source-file links below refer to the repository layout; source files and test
drivers are not included in the binary ZIPs.

## Current parser support

The [statement parser](src/program.c) accepts `mov <identifier>, <expression>;`.
For example, save this as `example.asm`:

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

This example prints the same encoded bytes shown above. The semicolons terminate the
instructions; `//` and `/* ... */` introduce comments. Block comments do not nest.

Instruction names are case-sensitive: only lowercase `mov` is recognized.
The parser accepts an identifier as the destination; semantic validation then
requires lowercase `eax`. Each statement requires a semicolon, including the last
one. Statements can share a line or be separated by newlines and blank lines;
a trailing newline is optional. Line breaks within an instruction are not
supported, except inside block comments.

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

MOVs retain their source order in one flat
statement array; blocks do not create scopes or separate AST nodes. Each MOV
still needs its semicolon, but a closing brace takes no semicolon.

Blocks may nest up to `MAX_BLOCK_DEPTH`, currently 16 in
[include/program.h](include/program.h). This guard bounds recursive parser
calls to protect the C call stack. A 17th nested block reports
`blocks nested too deeply`. Missing and extra closing braces report
`expected closing brace` and `unexpected closing brace`, respectively.

The [expression parser](src/expr.c) supports integer literals, binary `+` and
`-`, multiplication (`*`), and parentheses. Multiplication binds more tightly
than addition and subtraction; operators at the same precedence associate
left to right. Thus `2+3*4` parses as `2+(3*4)`, while `(2+3)*4` groups the
addition first. Parentheses may nest up to 32 levels. Unary signs, symbols,
division, and other expression operators are not supported yet.

Each statement references its expression's root in the parser's node array.
Statement storage grows dynamically, starting at 16 entries and doubling as
needed; there is no fixed 256-statement limit. The source loader's 4096-byte
file limit still applies.

The CLI exits with status 0 after successful encoding,
and 1 on a loading, lexing, parsing, semantic, or allocation error, or incorrect command-line
usage. Diagnostics go to stderr.

## Semantic checking and evaluation

The [semantic checker](src/semantic.c) evaluates expression nodes in dependency
order, then validates each MOV and stores its immediate in `Statement.value`.
For example:

```asm
mov eax, (10+4)*3;
```

Produces:

```text
B8 2A 00 00 00
```

Only `eax` is supported. Every literal and intermediate expression result must
fit the signed 32-bit range `-2147483648..2147483647`. The final MOV immediate
must also be nonnegative, giving an accepted range of `0..2147483647`.
These are the current language restrictions. The lexer's larger literal range
does not bypass semantic checking.

Examples rejected by the semantic checker:

| Input | Diagnostic |
| --- | --- |
| `mov ebx, 7;` | `only register eax is supported` |
| `mov eax, 1-2;` | `mov immediate must be nonnegative in this language` |
| `mov eax, 2147483647+1;` | `expression outside signed 32-bit range` |

Intermediate results are checked too: `mov eax, (2147483647+1)-1;` is rejected
even though its final mathematical result would fit. Evaluation computes values
for later encoding; it does not execute instructions or modify CPU registers.

## Encoding and current limitations

The encoder emits five bytes per MOV: opcode `B8`, followed by the evaluated
immediate as four bytes in little-endian order. For example, 42 becomes
`2A 00 00 00`. The byte buffer grows dynamically as instructions are appended.

The current language supports only MOV to `eax`. Other instructions and
registers, labels, symbol resolution, memory operands, directives, and object
or executable file formats are not implemented. The immediate range remains
`0..2147483647`, as enforced by semantic checking. Blocks provide grouping,
not scope or control flow. Empty input or empty blocks produce only a newline.

## Current lexer support

The implementation is in [src/lexer.c](src/lexer.c), with the public token types
and API in [include/lexer.h](include/lexer.h).

### Identifiers

Identifiers follow `[A-Za-z_][A-Za-z0-9_]*`, for example `mov`, `R0`, `_start`,
and `label_2`. All names produce `TK_IDENT`; there are no dedicated tokens for
instructions, registers, directives, or keywords. Spelling is preserved, and the
`token_is()` helper compares it case-sensitively.

### Integer literals

All supported integer forms produce `TK_NUMBER` with a decoded `long long` value.

| Base | Syntax | Example | Value |
| --- | --- | --- | --- |
| Decimal | Digits beginning with `1` through `9` | `42` | 42 |
| Hexadecimal | `0x` or `0X`, then one or more hexadecimal digits | `0x2A` | 42 |
| Binary | `0b` or `0B`, then one or more binary digits | `0b101010` | 42 |
| Octal | Leading `0`, followed by zero or more octal digits | `052` | 42 |

`0` is valid and has value zero. Hexadecimal digits accept both letter cases.
Values must fit in `0..LLONG_MAX` (9223372036854775807 on the current targets).
Signs are separate tokens: `-42` becomes `TK_MINUS`, then `TK_NUMBER(42)`.
Consequently, the magnitude in `-9223372036854775808` exceeds the literal limit
and is rejected before parsing.

The lexer consumes consecutive ASCII letters, digits, and underscores after a
number starts, and rejects the whole literal if any are invalid for its base.
Examples of rejected literals include `0x`, `0b`, `0b102`, `08`, `0xGG`, `123abc`,
and `1_000`. Numeric suffixes, digit separators, floating-point literals, and
an explicit `0o` octal prefix are not supported. Punctuation can directly follow
a number: `42+7` produces three tokens.

### Punctuation

| Character | Token kind |
| --- | --- |
| `+` | `TK_PLUS` |
| `-` | `TK_MINUS` |
| `*` | `TK_STAR` |
| `(` | `TK_LPAREN` |
| `)` | `TK_RPAREN` |
| `,` | `TK_COMMA` |
| `;` | `TK_SEMI` |
| `{` | `TK_LBRACE` |
| `}` | `TK_RBRACE` |
| `:` | `TK_COLON` |

The parser uses arithmetic operators, parentheses, commas, semicolons, and braces.
Colons are tokenized but not accepted by the current grammar.
For example, `label:` lexes as an identifier and a colon, but does not yet
define a label.
Other punctuation is rejected, including a standalone `/`, `.`, `=`, `[` and `]`.
String and character literals are not supported.

### Newlines, whitespace, and comments

- Spaces, tabs, vertical tabs, and form feeds are skipped.
- LF, CRLF, and lone CR produce `TK_NEWLINE` (token ID 13). CRLF is one token
  spanning two bytes; blank lines also produce newline tokens. No newline is
  inserted at EOF.
- `//` starts a comment that ends before LF, CRLF, lone CR, or EOF. The
  terminating newline remains available as a token.
- `/* ... */` comments can span lines and end at the first `*/`; they do not
  nest. Newlines inside block comments are skipped with the comment.
- Comments produce no tokens and can touch other tokens. `a/**/b` produces two
  identifiers.
- A semicolon is a `TK_SEMI` token, **not a comment marker**.

An unterminated `/*` comment is an error, with a span from the opening slash
through EOF. Adjacent comments are skipped iteratively, without recursive calls.

### Source limits, token spans, and errors

The [source loader](src/source.c) accepts at most 4096 non-NUL ASCII bytes per
file. It rejects embedded NUL bytes and non-ASCII input, including a UTF-8 BOM.
Files are read in binary mode, preserving their original byte offsets.

Each `Token` contains its kind, a zero-based half-open byte span `[start, end)`,
and a numeric value. The span selects the original spelling in `Source.text`;
it excludes skipped whitespace and comments. A `TK_NEWLINE` span covers the
original line-ending bytes. Non-number tokens have value zero. `TK_END` marks
EOF with an empty span.

An invalid character, malformed integer, or integer overflow sets `Lexer.failed`
and reports `invalid character or integer out of range`. An unclosed block
comment sets the same failure flag and reports `unterminated block comment`.
Diagnostics go to stderr and include the file path, one-based line and column,
and byte span. LF, CRLF, and lone CR each advance the line count once. Columns
count bytes; tabs advance one column. Lexing stops on the first error, with no
error recovery.

### Using the lexer from a parser

- `lexer_start(&lexer, &source)` initializes the lexer and reads the first token.
- Inspect `lexer.token`, then call `lexer_next(&lexer)` to advance. Stop when
  `lexer.failed` is set or the token kind is `TK_END`.
- `token_is(&source, token, "word")` compares the token's source text exactly;
  it does not check the token kind, so check `TK_IDENT` separately when needed.
- Keep the `Source` alive while using the lexer and resolving token spans.

Check `failed` before using a token: a malformed number still has kind
`TK_NUMBER`, while invalid punctuation and unterminated block comments leave
kind `TK_END`.
Calling `lexer_next()` after failure returns `TK_END` without resuming scanning.

The statement parser skips `TK_NEWLINE` between statements and blocks at every
nesting level.
Block comments act as whitespace, including when they span multiple lines.
EOF is accepted after the last semicolon or closing brace without a trailing newline.

### Inspecting tokens

With `BUILD_TESTING=ON`, the dedicated lexer test driver prints tokens:

```powershell
.\build\windows-debug\Debug\lexer_test_driver.exe tokens.asm
```

On Linux, use `./build/GCC-debug/lexer_test_driver tokens.asm`.
For a file containing `label: 42+7;` followed by an LF newline, the driver
prints:

```text
token 11 [0,5) value=0
token 10 [5,6) value=0
token 12 [7,9) value=42
token 1 [9,10) value=0
token 12 [10,11) value=7
token 7 [11,12) value=0
token 13 [12,13) value=0
```

The numeric token IDs come from the current `TokenKind` enum. EOF is not printed.
With CRLF, the final newline span is `[12,14)`; without a trailing newline,
the final `token 13` line is absent.
The driver exits with status 0 on successful lexing and 1 on a loading or lexing
error (or incorrect command-line usage).

## Building and testing

### Reusable example encoding test

`tests/encode_file.cmake` assembles an existing file and checks that the raw
`program.bin` bytes match stdout. Pass `EXPECTED_HEX` to also check the expected
instruction encoding. Run from the repository root after building:

```powershell
cmake "-DKASM=build/windows-debug/Debug/kasm.exe" "-DSOURCE=examples/program.asm" "-DEXPECTED_HEX=B8 2A 00 00 00 C3" -P tests/encode_file.cmake
```

Change `SOURCE` to reuse the script with another example; omit `EXPECTED_HEX`
when you only want to check successful assembly and binary/stdout consistency.
By default, each source path gets its own directory under `build/example-tests`.
The script prints the resulting binary path. Repeating the same example replaces
its previous binary. You can override the directory with `-DOUTPUT_DIR=<path>`;
use different directories for examples that run concurrently.

CTest uses this script for `examples/mov_42.asm`, `examples/ret.asm`, and
`examples/program.asm`, saving separate binaries under
`build/windows-debug/examples/Debug/<test-name>/program.bin` with the Windows
Debug preset. To register another example, add a call inside `BUILD_TESTING`:

```cmake
add_example_test(my_example examples/my_example.asm "B8 07 00 00 00 C3")
```

### Configure and run the suite

Build from a source checkout with CMake and a C compiler. The Windows presets
target Visual Studio 2026 with the C++ build tools installed. The Linux/WSL2
preset uses GCC and Make. Use a CMake version that supports your generator and
the repository's version-8 preset file; the basic CMake project requires 3.20
or newer when configuring without presets.

The eleven CTest tests cover:

- Exact MOV encoding: `mov eax,42;` produces `B8 2A 00 00 00`.
- Semantic evaluation of `mov eax, (10+4)*3;` to `42`.
- Exact expression ASTs for integers, addition/subtraction, multiplication
  precedence, and parentheses overriding precedence.
- Multiple MOV statements with LF and CRLF line endings, binary literals in
  instructions, evaluated values (including inside nested blocks), and a
  missing-semicolon diagnostic.
- Dynamic storage growth to 300 MOV statements, preserving their operands.
- Empty, nested, and sibling blocks; blank lines within blocks; statement order
  and expression references; missing/extra brace diagnostics; and acceptance at
  `MAX_BLOCK_DEPTH` with rejection one level beyond it.
- Integer literal lexing, comments, newline tokens, LF/CRLF/CR line endings,
  unterminated-comment diagnostics, and long sequences of adjacent comments.

Expression tests use `expr_test_driver` to inspect ASTs independently of the
encoding CLI. Semantic and multiple-statement tests use `semantic_test_driver`
to inspect statement counts and evaluated values. Lexer tests use
`lexer_test_driver`. These drivers are built only when testing is enabled.

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
The presets create separate build directories under `build`; CTest must point to
the configured directory. For Release, use `windows-release` in all three commands
and replace `-C Debug` with `-C Release`.

If your terminal is already in `build/GCC-debug`, run `cd ../..` first to return
to the repository root before using the commands above.

The commands include `-V` for verbose output even when tests pass; omit it for
a shorter report.

After adding a new source file, rerun the configure command before building
so the generated project includes it.

## Packaging and releases

The repository workflow builds and tests Debug and Release configurations on
Windows x64 and Linux x64. On a push to `main`, successful builds produce two
Release ZIPs through CPack and publish a GitHub Release. Tags use
`v<version>-build.<run-number>`; the version comes from `project(kasm VERSION ...)`
in `CMakeLists.txt`. A merge to `main` triggers this process through its push.

To build a Windows ZIP locally from the repository root:

```powershell
cmake --preset windows-release -DBUILD_TESTING=ON
cmake --build --preset windows-release
ctest --test-dir build/windows-release -C Release --output-on-failure
cpack --config build/windows-release/CPackConfig.cmake -C Release -G ZIP -B dist
```

The package installs the executable under `bin` and documentation under
`share/doc/kasm`. GitHub's automatically generated source archives are separate
from these executable packages.

## License

Copyright 2026 Kenneth Looney. Licensed under the Apache License, Version 2.0.
See [LICENSE](LICENSE), included beside this README in release packages.

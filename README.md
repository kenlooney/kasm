# Ken's Assembler

Kasm currently loads a source file and prints its lexer tokens. AST construction,
parsing, instruction validation, and machine-code generation are not implemented yet.

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

These tokens have no grammatical meaning assigned yet. For example, `label:`
lexes as an identifier and a colon, but does not yet define a label.
Other punctuation is rejected, including a standalone `/`, `.`, `=`, `[` and `]`.
String and character literals are not supported.

### Whitespace and comments

- Spaces, tabs, carriage returns, line feeds, vertical tabs, and form feeds are
  skipped. There is no newline token or preserved whitespace.
- `//` starts a comment that ends at a line feed (`\n`) or EOF.
- `/* ... */` comments can span lines and end at the first `*/`; they do not nest.
- Comments produce no tokens and can touch other tokens. `a/**/b` produces two
  identifiers.
- A semicolon is a `TK_SEMI` token, **not a comment marker**.

Current limitation: an unterminated `/*` comment is silently consumed through
EOF. Also, a lone carriage return does not end a `//` comment. LF and CRLF line
endings are supported; both bytes of CRLF count toward source offsets.

### Source limits, token spans, and errors

The [source loader](src/source.c) accepts at most 4096 non-NUL ASCII bytes per
file. It rejects embedded NUL bytes and non-ASCII input, including a UTF-8 BOM.
Files are read in binary mode, preserving their original byte offsets.

Each `Token` contains its kind, a zero-based half-open byte span `[start, end)`,
and a numeric value. The span selects the original spelling in `Source.text`;
it excludes surrounding whitespace and comments. Non-number tokens have value
zero. `TK_END` marks EOF with an empty span.

An invalid character, malformed integer, or integer overflow sets `Lexer.failed`
and reports `invalid character or integer out of range` to stderr, including the
file path, one-based line and column, and byte span. Columns count bytes; tabs
advance one column. Lexing stops on the first error, with no error recovery.

### Using the lexer from a parser

- `lexer_start(&lexer, &source)` initializes the lexer and reads the first token.
- Inspect `lexer.token`, then call `lexer_next(&lexer)` to advance. Stop when
  `lexer.failed` is set or the token kind is `TK_END`.
- `token_is(&source, token, "word")` compares the token's source text exactly;
  it does not check the token kind, so check `TK_IDENT` separately when needed.
- Keep the `Source` alive while using the lexer and resolving token spans.

Check `failed` before using a token: a malformed number still has kind
`TK_NUMBER`, and an invalid punctuation character leaves kind `TK_END`.
Calling `lexer_next()` after failure returns `TK_END` without resuming scanning.
Because newlines are discarded, a future parser cannot use a newline token to
separate statements without changing the lexer or inspecting the source gaps.

### Inspecting tokens

Run the built executable with one source-file path, for example:

```powershell
.\build\windows-debug\Debug\kasm.exe example.asm
```

For a file containing exactly `label: 42+7;`, the current CLI prints:

```text
token 11 [0,5) value=0
token 10 [5,6) value=0
token 12 [7,9) value=42
token 1 [9,10) value=0
token 12 [10,11) value=7
token 7 [11,12) value=0
```

The numeric token IDs come from the current `TokenKind` enum. EOF is not printed.
The program exits with status 0 on successful lexing and 1 on a loading or lexing
error (or incorrect command-line usage).

### Run Tests
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

To see test output even when tests pass, add `-V` to the command.

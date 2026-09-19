# The lexer

The implementation is in
[src/lexer.c](https://github.com/kenlooney/kasm/blob/main/src/lexer.c), with
the public token types and API in
[include/lexer.h](https://github.com/kenlooney/kasm/blob/main/include/lexer.h).

## Identifiers

Identifiers follow `[A-Za-z_][A-Za-z0-9_]*`, for example `mov`, `R0`,
`_start`, and `label_2`. All names produce `TK_IDENT`; there are no
dedicated tokens for instructions, registers, directives, or keywords.
Spelling is preserved, and the `token_is()` helper compares it
case-sensitively.

## Integer literals

All supported integer forms produce `TK_NUMBER` with a decoded `long long`
value.

| Base | Syntax | Example | Value |
| --- | --- | --- | --- |
| Decimal | Digits beginning with `1` through `9` | `42` | 42 |
| Hexadecimal | `0x` or `0X`, then one or more hexadecimal digits | `0x2A` | 42 |
| Binary | `0b` or `0B`, then one or more binary digits | `0b101010` | 42 |
| Octal | Leading `0`, followed by zero or more octal digits | `052` | 42 |

`0` is valid and has value zero. Hexadecimal digits accept both letter
cases. Values must fit in `0..LLONG_MAX` (9223372036854775807 on the
current targets). Signs are separate tokens: `-42` becomes `TK_MINUS`, then
`TK_NUMBER(42)`. Consequently, the magnitude in `-9223372036854775808`
exceeds the literal limit and is rejected before parsing.

The lexer consumes consecutive ASCII letters, digits, and underscores
after a number starts, and rejects the whole literal if any are invalid
for its base. Examples of rejected literals include `0x`, `0b`, `0b102`,
`08`, `0xGG`, `123abc`, and `1_000`. Numeric suffixes, digit separators,
floating-point literals, and an explicit `0o` octal prefix are not
supported. Punctuation can directly follow a number: `42+7` produces three
tokens.

## Punctuation

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

The parser uses arithmetic operators, parentheses, commas, semicolons, and
braces. The statement parser recognizes an identifier followed by a colon
as a label definition, for example `label:`. A label name can also be the
operand of `jmp`. Other punctuation is rejected, including a standalone
`/`, `.`, `=`, `[` and `]`. String and character literals are not
supported.

## Newlines, whitespace, and comments

- Spaces, tabs, vertical tabs, and form feeds are skipped.
- LF, CRLF, and lone CR produce `TK_NEWLINE` (token ID 13). CRLF is one
  token spanning two bytes; blank lines also produce newline tokens. No
  newline is inserted at EOF.
- `//` starts a comment that ends before LF, CRLF, lone CR, or EOF. The
  terminating newline remains available as a token.
- `/* ... */` comments can span lines and end at the first `*/`; they do
  not nest. Newlines inside block comments are skipped with the comment.
- Comments produce no tokens and can touch other tokens. `a/**/b` produces
  two identifiers.
- A semicolon is a `TK_SEMI` token, **not a comment marker**.

An unterminated `/*` comment is an error, with a span from the opening
slash through EOF. Adjacent comments are skipped iteratively, without
recursive calls.

## Source limits, token spans, and errors

The [source loader](https://github.com/kenlooney/kasm/blob/main/src/source.c)
stores input in a dynamically growing buffer. There is no project-defined
source-byte limit; practical limits are available memory and the
platform's allocation limits. It rejects embedded NUL bytes and non-ASCII
input, including a UTF-8 BOM. Files are read in binary mode, preserving
their original byte offsets.

Each `Token` contains its kind, a zero-based half-open byte span
`[start, end)`, and a numeric value. The span selects the original
spelling in `Source.text`; it excludes skipped whitespace and comments. A
`TK_NEWLINE` span covers the original line-ending bytes. Non-number tokens
have value zero. `TK_END` marks EOF with an empty span.

An invalid character, malformed integer, or integer overflow sets
`Lexer.failed` and reports `invalid character or integer out of range`. An
unclosed block comment sets the same failure flag and reports
`unterminated block comment`. Diagnostics go to stderr and include the
file path, one-based line and column, and byte span. LF, CRLF, and lone CR
each advance the line count once. Columns count bytes; tabs advance one
column. Lexing stops on the first error, with no error recovery.

## Using the lexer from a parser

- `lexer_start(&lexer, &source)` initializes the lexer and reads the first
  token.
- Inspect `lexer.token`, then call `lexer_next(&lexer)` to advance. Stop
  when `lexer.failed` is set or the token kind is `TK_END`.
- `token_is(&source, token, "word")` compares the token's source text
  exactly; it does not check the token kind, so check `TK_IDENT`
  separately when needed.
- Keep the `Source` alive while using the lexer and resolving token spans.

Check `failed` before using a token: a malformed number still has kind
`TK_NUMBER`, while invalid punctuation and unterminated block comments
leave kind `TK_END`. Calling `lexer_next()` after failure returns `TK_END`
without resuming scanning.

The statement parser skips `TK_NEWLINE` between statements and blocks at
every nesting level. Block comments act as whitespace, including when they
span multiple lines. EOF is accepted after the last semicolon or closing
brace without a trailing newline.

## Inspecting tokens

With `BUILD_TESTING=ON`, the dedicated lexer test driver prints tokens:

```powershell
.\build\windows-debug\Debug\lexer_test_driver.exe tokens.asm
```

On Linux, use `./build/GCC-debug/lexer_test_driver tokens.asm`. For a file
containing `label: 42+7;` followed by an LF newline, the driver prints:

```text
token 11 [0,5) value=0
token 10 [5,6) value=0
token 12 [7,9) value=42
token 1 [9,10) value=0
token 12 [10,11) value=7
token 7 [11,12) value=0
token 13 [12,13) value=0
```

The numeric token IDs come from the current `TokenKind` enum. EOF is not
printed. With CRLF, the final newline span is `[12,14)`; without a
trailing newline, the final `token 13` line is absent. The driver exits
with status 0 on successful lexing and 1 on a loading or lexing error (or
incorrect command-line usage).

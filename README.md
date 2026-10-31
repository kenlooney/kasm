# Ken's Assembler Framework

An extensible assembler framework for building assemblers for different CPUs and languages. Kasm is its first assembler, an x86 assembler in development. The current implementation provides expression parsing and statement splitting; instruction assembly and machine-code output are still to come.

## Kasm statements and comments

Kasm currently requires **one expression statement per physical line**. Newlines separate statements, and blank lines and comment-only lines are ignored. An expression cannot yet continue onto another line, including inside parentheses.

```asm
// One expression per line
2 + 3 * 4
(2 + 3) * 4  // Parentheses change the order
/* Block comments can span
   multiple lines. */
10 minus 3
```

Kasm supports `//` line comments and `/* ... */` block comments. A newline inside a block comment still separates statements. Currently, statements consist of expressions; instructions and labels are not yet handled by the statement parser.

## Kasm expressions

Kasm supports addition, subtraction, multiplication, and parentheses in expressions. Multiplication takes precedence over addition and subtraction; parentheses can change the order of evaluation.

| Operator | Default aliases |
|----------|-----------------|
| `+` | `add`, `plus` |
| `-` | `sub`, `minus` |
| `*` | `mul`, `times` |

For example, `2 plus 3 times 4` evaluates to `14`, while `(2 plus 3) times 4` evaluates to `20`. Expressions are parsed into abstract syntax trees (ASTs). The single-expression API can also evaluate a tree using unsigned 64-bit integers, reporting arithmetic overflow or subtraction underflow as errors. Building an AST alone does not evaluate it.

These aliases are defined in the `KasmParser` constructor and compiled into `kasmcore`. Other assemblers can define their own aliases in code using the framework's `AliasTable`.

## Using the framework

The project separates the reusable framework in `asmfw` from Kasm's lexer and parser in `kasmcore`. Link to `asmfw` to build another assembler, or link to `kasmcore` to use Kasm's components. `kasmcore` also links to `asmfw`.

`KasmParser::parse_statements()` accepts a source string and returns one `Expression` AST per statement. It reports the line number on a parse error. For example:

```cpp
KasmLexer lexer;
KasmParser parser(lexer);
std::vector<Expression> statements;
std::string error;

if (!parser.parse_statements("2 + 3 * 4\n(2 + 3) * 4\n", statements, error)) {
    // Handle the error.
}
```

The current `kasm` executable displays its operator aliases; it does not yet read and assemble a source file from the command line.

To build another assembler, implement a lexer and parser using the framework's base classes, then provide them to an `Assembler`. For example, Kasm's components can be connected like this:

```cpp
#include "kasmlexer.hpp"
#include "kasmparser.hpp"
#include "assembler.hpp"
#include <memory>
#include <utility>

int main() {
    auto lexer = std::make_unique<KasmLexer>();
    auto parser = std::make_unique<KasmParser>(*lexer);
    auto assembler = std::make_unique<Assembler>(
        "x86", "source code", std::move(lexer), std::move(parser));
    return 0;
}
```

## Building

The presets require CMake 3.28 or newer and vcpkg. Set `VCPKG_ROOT` to your vcpkg installation. If VS Code was already running, restart it so it picks up the environment variable.

### Windows

Install Visual Studio 2026 with the C++ desktop workload, then run:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Use the `windows-release` configure and build presets for a release build. Tests are included in debug builds.

### Linux / WSL2

Install GCC/G++, Make, and a Linux installation of vcpkg. Set `VCPKG_ROOT` to the Linux vcpkg directory, not the Windows installation. For example:

```sh
export VCPKG_ROOT="$HOME/vcpkg"
cmake --preset GCC-debug
cmake --build --preset GCC-debug
ctest --preset GCC-debug
```

## What's next?

I plan to build a graphical emulator using SDL3.

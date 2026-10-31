# Ken's Assembler Framework

An extensible assembler framework for building assemblers for different CPUs and languages. Kasm is its first assembler, an x86 assembler in development.

## Kasm expressions

Kasm supports addition, subtraction, multiplication, and parentheses in expressions. Multiplication takes precedence over addition and subtraction; parentheses can change the order of evaluation.

| Operator | Default aliases |
|----------|-----------------|
| `+` | `add`, `plus` |
| `-` | `sub`, `minus` |
| `*` | `mul`, `times` |

For example, `2 plus 3 times 4` evaluates to `14`, while `(2 plus 3) times 4` evaluates to `20`. Expressions currently use unsigned 64-bit integers and report arithmetic overflow or subtraction underflow as errors.

Kasm looks for an `ops.txt` file in the current working directory when a parser is created. Each line contains an operator followed by its aliases, separated by spaces. For example:

```text
+ add plus
- sub minus
* mul times
```

The table above shows the defaults used when an operator has no entry in the file. An entry in `ops.txt` supplies that operator's aliases; the operator symbol itself remains valid. The parser can also save its alias table explicitly.

## Using the framework

The project separates the reusable framework in `asmfw` from Kasm's lexer and parser in `kasmcore`. Link to `asmfw` to build another assembler, or link to `kasmcore` to use Kasm's components. `kasmcore` also links to `asmfw`.

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

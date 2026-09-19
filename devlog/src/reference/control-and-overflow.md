# Control instructions and overflow safety

## Operand-free control instructions

The instruction set also includes two operand-free forms:

| Instruction | Bytes |
| --- | --- |
| `halt` | `F4` |
| `pause` | `F3 90` |

Both require a semicolon and have fixed layout sizes. `pause` is
recognized as a two-byte sequence before a decoder reports an unknown
byte. The end-to-end test checks both the binary and decoded listing:

```powershell
ctest --test-dir build/windows-debug -C Debug -R "^encode.control$" --output-on-failure
```

## Overflow-safe constant expressions

Constant expressions are evaluated with checked signed `long long`
arithmetic before Kasm applies its signed 32-bit language limit. Addition,
subtraction, and multiplication now reject intermediate overflow instead
of relying on C signed overflow behavior. The semantic regression retains
the successful precedence case and checks overflowing examples for all
three operators:

```powershell
ctest --test-dir build/windows-debug -C Debug -R "^semantic.expression$" --output-on-failure
```

Symbol-valued data remains a future layout feature at the time this
checked-arithmetic behavior was added; as of 0.33.0, data directives can
also resolve label expressions after layout — see
[Data directives and layout expressions](data-directives.md).

# 0.36.0 — ELF64 Executables

Version 0.36.0 adds Kasm's first standalone executable output. The Windows
build can create the file, while Linux or WSL loads and executes it.

## From payload offsets to virtual addresses

Raw output only needs an ordered byte sequence. ELF must also tell Linux where
to map those bytes and where execution begins. Kasm writes a 64-byte ELF header,
one 56-byte `PT_LOAD` program header, padding through file offset `0x1000`,
and the encoded instruction payload.

The mapping begins at virtual address `0x400000`. For an assembly label at
payload offset *n*, the entry address is:

```text
0x400000 + 0x1000 + n
```

For `_start` at payload offset zero, this is `0x401000`. The twelve-byte
`exit42.asm` payload makes the complete file `0x100C` bytes long.

## Explicit CLI selection

```powershell
.\build\windows-debug\Debug\kasm.exe --format elf --entry _start -o build\exit42 examples\exit42.asm
```

`--entry` names the label where Linux starts. This differs from COFF's
`--export`: an ELF entry controls process startup, while a COFF export gives
a linker a visible symbol.

The first ELF profile is deliberately small: Linux x86-64, one
instruction-only read/execute mapping, no section table, no runtime loader
patches, no interpreter, and no dynamic dependencies. It uses Kasm's dynamic
`Bytes` storage and configured image limit rather than a fixed fixture.

## Validation

The permanent `elf.cli` test checks the ELF identity, machine and executable
type, entry address, program-header dimensions, segment addresses and flags,
file and memory sizes, alignment, and exact payload at offset `0x1000`.
Linux builds also execute the result and require status 42. Negative cases
cover a missing entry, an entry at end-of-image, data, and loader patches.

Manual inspection with `readelf -h -l` confirms one x86-64 `LOAD` segment
with read/execute permissions, entry `0x401000`, and size `0x100C`.

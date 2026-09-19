# Executable output

## Minimal Linux ELF64

Kasm can write a standalone Linux x86-64 executable:

```powershell
.\build\windows-debug\Debug\kasm.exe --format elf --entry _start -o build\exit42 examples\exit42.asm
```

The source for the first executable is:

```asm
_start:
mov eax, 60;
mov edi, 42;
syscall;
```

The output may be created on Windows, but it runs under Linux x86-64. Inspect
and execute it in Linux or WSL:

```sh
readelf -h -l build/exit42
chmod +x build/exit42
./build/exit42
test $? -eq 42
```

The writer places code at file offset `0x1000`, maps the file from virtual
address `0x400000`, and converts the selected label's payload offset into an
entry virtual address. There is one read/execute `PT_LOAD` segment. The file
has no section table, interpreter, dynamic dependencies, or C runtime.

The current profile accepts a nonempty instruction-only 64-bit image. The
entry label must point inside the payload. Data directives and load-time
patches are rejected until later executable layouts describe them correctly.

## COFF objects are different

`--format coff --export <label>` produces linker input. It preserves symbols
and relocation records for another tool to finish. `--format elf --entry
<label>` produces a loadable process image and identifies its first
instruction. Both require an explicit `-o <output-path>`.

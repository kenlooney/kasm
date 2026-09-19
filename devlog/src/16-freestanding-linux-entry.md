# 0.35.0 — A Freestanding Linux Entry Sequence

Version 0.35.0 adds the two source forms needed to express a minimal Linux
x86-64 process entry point: `mov edi, <immediate>;` and `syscall;`.

## A process entry point is not a called function

Earlier executable-memory examples ended with `ret;` because a C runner had
called the generated code and placed a return address on the stack. Linux
enters a freestanding executable at `_start` without making that function
call. The initial stack contains process startup information, so treating its
first value as a return address would be incorrect.

[`examples/exit42.asm`](https://github.com/kenlooney/kasm/blob/main/examples/exit42.asm)
therefore requests process termination directly:

```asm
_start:
mov eax, 60;
mov edi, 42;
syscall;
```

On Linux x86-64, RAX selects the system call and RDI holds its first argument.
Writing EAX and EDI clears the upper halves of their 64-bit registers. The
sequence encodes as:

```text
B8 3C 00 00 00 BF 2A 00 00 00 0F 05
```

This milestone defines and verifies the payload contract. A later executable
writer will place it in ELF64 and select `_start` as the image entry point.

## Changes across the pipeline

Semantic checking accepts EDI only as a 32-bit MOV-immediate destination in
64-bit mode and assigns register code 7. The encoder maps that code to BF; the
decoder handles BF explicitly rather than indexing past its register table.

SYSCALL has its own `ST_SYSCALL` statement kind. The parser requires
`syscall;`, semantic checking restricts it to 64-bit mode, layout reserves
two bytes, the encoder writes `0F 05`, and the decoder matches both bytes.

## Validation

The permanent `encode.exit42` test checks all twelve bytes and the decoded
listing. Three negative tests reject `syscall 1;`, 16-bit SYSCALL, and
16-bit `mov edi,42;`. The complete Windows suite passes 44 tests.

This sequence must not run through Kasm's C runner: syscall 60 terminates the
surrounding Linux process. Version 0.35.0 adds the instructions, not ELF output.

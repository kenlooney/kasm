# KASM x86 Instruction Roadmap
## Complete Path to Follow
```
source text
→ parser statement kind
→ semantic validation
→ layout size
→ emitted bytes
→ KEMU behavior
→ tests
```
## Implemented instruction reference

These instructions currently pass through both KASM's assembly pipeline and
KEMU's decoder. The syntax column shows how they are written in KASM source.

| Instruction | KASM syntax | Encoding | Name and meaning | CPU effect |
| --- | --- | --- | --- | --- |
| `MOV AX, imm16` | `mov ax, expression;` | `B8 iw` | Move a 16-bit immediate value into `AX` | Replaces `AX`; flags are unchanged |
| `NOP` | `nop;` | `90` | No Operation | Advances `IP` without otherwise changing CPU state |
| `CLI` | `cli;` | `FA` | Clear Interrupt Flag | Clears `IF`, disabling maskable interrupts |
| `STI` | `sti;` | `FB` | Set Interrupt Flag | Sets `IF`, enabling maskable interrupts |
| `HLT` | `hlt;` | `F4` | Halt | Places the CPU in its halted state until a qualifying wake event |
| `CLC` | `clc;` | `F8` | Clear Carry Flag | Clears `CF` |
| `STC` | `stc;` | `F9` | Set Carry Flag | Sets `CF` |
| `CMC` | `cmc;` | `F5` | Complement Carry Flag | Toggles `CF` |
| `CLD` | `cld;` | `FC` | Clear Direction Flag | Clears `DF`; string operations advance toward higher addresses |
| `STD` | `std;` | `FD` | Set Direction Flag | Sets `DF`; string operations advance toward lower addresses |
| `JMP rel8` | `jmp8 label;` | `EB cb` | Jump using a signed 8-bit relative displacement | Adds the displacement to `IP` after the instruction has been fetched |

Encoding notation used above:

- `iw` is an immediate 16-bit word stored in little-endian byte order.
- `cb` is a signed 8-bit relative displacement measured from the end of the
  jump instruction.
- `CF`, `IF`, and `DF` mean carry flag, interrupt flag, and direction flag.

KEMU is intentionally a partial emulator. For example, `HLT` records the
halted state and ends the current driver loop; interrupt wake-up behavior is
not modeled yet. Likewise, direction-flag behavior is stored correctly even
though string instructions that consume `DF` are not implemented yet.

## No-operand, single-byte instructions

- [x] `CLI` — `FA`
- [x] `STI` — `FB`
- [x] `HLT` — `F4`
- [x] `NOP` — `90`
- [x] `CLC` — `F8`
- [x] `STC` — `F9`
- [x] `CMC` — `F5`
- [x] `CLD` — `FC`
- [x] `STD` — `FD`
- [x] `LAHF` — `9F`
- [x] `SAHF` — `9E`
- [ ] `PUSHF` — `9C`
- [ ] `POPF` — `9D`
- [ ] `CBW` — `98`
- [ ] `CWD` — `99`
- [ ] `IRET` — `CF`

## Single-byte opcode with encoded register

- [ ] `INC r16` — `40+rw`
- [ ] `DEC r16` — `48+rw`
- [ ] `PUSH r16` — `50+rw`
- [ ] `POP r16` — `58+rw`
- [ ] `XCHG AX,r16` — `90+rw`
- [x] `MOV AX,imm16` — `B8 iw`
- [ ] `MOV r16,imm16` — `B8+rw iw`

## Opcode plus immediate or relative operand

- [x] `JMP rel8` — `EB cb`
- [ ] `JMP rel16` — `E9 cw`
- [ ] `CALL rel16` — `E8 cw`
- [ ] `INT imm8` — `CD ib`
- [ ] `RET` — `C3`
- [ ] `RET imm16` — `C2 iw`

## Conditional short jumps

- [ ] `JO` — `70 cb`
- [ ] `JNO` — `71 cb`
- [ ] `JB`/`JC` — `72 cb`
- [ ] `JAE`/`JNC` — `73 cb`
- [ ] `JE`/`JZ` — `74 cb`
- [ ] `JNE`/`JNZ` — `75 cb`
- [ ] `JBE` — `76 cb`
- [ ] `JA` — `77 cb`
- [ ] `JS` — `78 cb`
- [ ] `JNS` — `79 cb`
- [ ] `JP` — `7A cb`
- [ ] `JNP` — `7B cb`
- [ ] `JL` — `7C cb`
- [ ] `JGE` — `7D cb`
- [ ] `JLE` — `7E cb`
- [ ] `JG` — `7F cb`

## Requires ModR/M support

- [ ] General register-to-register `MOV`
- [ ] `ADD`
- [ ] `SUB`
- [ ] `CMP`
- [ ] `AND`
- [ ] `OR`
- [ ] `XOR`
- [ ] Memory operands

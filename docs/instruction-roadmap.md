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
| `INC r16` | `inc register;` | `40+rw` | Increment a 16-bit register | Adds one; updates arithmetic status flags except `CF` |
| `DEC r16` | `dec register;` | `48+rw` | Decrement a 16-bit register | Subtracts one; updates arithmetic status flags except `CF` |
| `PUSH r16` | `push register;` | `50+rw` | Push a 16-bit register | Decrements `SP` by two and stores the register at `SS:SP` |
| `POP r16` | `pop register;` | `58+rw` | Pop into a 16-bit register | Loads the register from `SS:SP` and increments `SP` by two |
| `XCHG AX, r16` | `xchg ax, register;` | `90+rw` | Exchange `AX` with a 16-bit register | Swaps the register values; flags are unchanged |
| `MOV r16, imm16` | `mov register, expression;` | `B8+rw iw` | Move a 16-bit immediate into a register | Replaces the register; flags are unchanged |
| `MOV Sreg, r/m16` | `mov segment, ax;` | `8E /r` | Move `AX` into a segment register | Replaces `ES`, `SS`, or `DS`; flags are unchanged |
| `NOP` | `nop;` | `90` | No Operation | Advances `IP` without otherwise changing CPU state |
| `CLI` | `cli;` | `FA` | Clear Interrupt Flag | Clears `IF`, disabling maskable interrupts |
| `STI` | `sti;` | `FB` | Set Interrupt Flag | Sets `IF`, enabling maskable interrupts |
| `HLT` | `hlt;` | `F4` | Halt | Places the CPU in its halted state until a qualifying wake event |
| `CLC` | `clc;` | `F8` | Clear Carry Flag | Clears `CF` |
| `STC` | `stc;` | `F9` | Set Carry Flag | Sets `CF` |
| `CMC` | `cmc;` | `F5` | Complement Carry Flag | Toggles `CF` |
| `CLD` | `cld;` | `FC` | Clear Direction Flag | Clears `DF`; string operations advance toward higher addresses |
| `STD` | `std;` | `FD` | Set Direction Flag | Sets `DF`; string operations advance toward lower addresses |
| `LAHF` | `lahf;` | `9F` | Load AH from Flags | Copies `SF`, `ZF`, `AF`, `PF`, and `CF` into `AH` using the architectural fixed-bit layout |
| `SAHF` | `sahf;` | `9E` | Store AH into Flags | Copies the status-flag bits in `AH` into `SF`, `ZF`, `AF`, `PF`, and `CF` |
| `PUSHF` | `pushf;` | `9C` | Push Flags | Decrements `SP` by two and stores FLAGS at `SS:SP` |
| `POPF` | `popf;` | `9D` | Pop Flags | Restores FLAGS from `SS:SP` and increments `SP` by two |
| `CBW` | `cbw;` | `98` | Convert Byte to Word | Sign-extends `AL` into `AX` |
| `CWD` | `cwd;` | `99` | Convert Word to Doubleword | Sign-extends `AX` into the `DX:AX` register pair |
| `IRET` | `iret;` | `CF` | Interrupt Return | Pops `IP`, `CS`, and FLAGS from the stack in that order |
| `JMP rel8` | `jmp8 label;` | `EB cb` | Jump using a signed 8-bit relative displacement | Adds the displacement to `IP` after the instruction has been fetched |
| `JMP ptr16:16` | `jmpfar segment, label;` | `EA cd` | Immediate far jump | Replaces `CS:IP` with the encoded segment and absolute offset |

Encoding notation used above:

- `iw` is an immediate 16-bit word stored in little-endian byte order.
- `rw` is the three-bit code for a 16-bit general-purpose register.
- `/r` is a ModR/M byte; the current segment-MOV form requires register mode,
  encodes `ES`, `SS`, or `DS` in its `reg` field, and selects `AX` with `r/m=0`.
- `cb` is a signed 8-bit relative displacement measured from the end of the
  jump instruction.
- `cd` is a four-byte far pointer stored as a little-endian 16-bit offset
  followed by a little-endian 16-bit segment.
- `CF`, `PF`, `AF`, `ZF`, `SF`, `IF`, and `DF` mean carry, parity, auxiliary
  carry, zero, sign, interrupt, and direction flag.

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
- [x] `PUSHF` — `9C`
- [x] `POPF` — `9D`
- [x] `CBW` — `98`
- [x] `CWD` — `99`
- [x] `IRET` — `CF`

## Single-byte opcode with encoded register

- [x] `INC r16` — `40+rw`
- [x] `DEC r16` — `48+rw`
- [x] `PUSH r16` — `50+rw`
- [x] `POP r16` — `58+rw`
- [x] `XCHG AX,r16` — `90+rw`
- [x] `MOV AX,imm16` — `B8 iw`
- [x] `MOV r16,imm16` — `B8+rw iw`

## ModR/M instructions

- [x] KASM encoding and KEMU decoding for `MOV ES/SS/DS,AX` — `8E /r`
- [ ] Automated KEMU execution coverage from a nonzero `AX` value

## Opcode plus immediate or relative operand

- [x] `JMP rel8` — `EB cb`
- [x] `JMP ptr16:16` — `EA cd`
- [ ] `JMP rel16` — `E9 cw`
- [ ] `CALL rel16` — `E8 cw`
- [ ] `INT imm8` — `CD ib`
- [ ] `RET` — `C3`
- [ ] `RET imm16` — `C2 iw`

## Conditional short jumps

- [ ] `JO` — `70 cb`
- [ ] `JNO` — `71 cb`
- [x] `JB`/`JC` — `72 cb`
- [x] `JAE`/`JNC` — `73 cb`
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

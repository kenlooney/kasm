#include <stdio.h>
#include <stdint.h>

#define CPU16_MEMORY_SIZE (1024u * 1024u)
#define CPU16_FLAG_CF (1u << 0)
#define CPU16_FLAG_FIXED (1u << 1)
#define CPU16_FLAG_PF (1u << 2)
#define CPU16_FLAG_AF (1u << 4)
#define CPU16_FLAG_ZF (1u << 6)
#define CPU16_FLAG_SF (1u << 7)
#define CPU16_FLAG_TF (1u << 8)
#define CPU16_FLAG_IF (1u << 9)
#define CPU16_FLAG_DF (1u << 10)
#define CPU16_FLAG_OF (1u << 11)

typedef struct
{
    unsigned char memory[CPU16_MEMORY_SIZE];
    uint32_t image_start;
    size_t image_size;
    uint16_t ax, bx, cx, dx;
    uint16_t sp, bp, si, di;
    uint16_t cs, ds, es, ss, ip;
    uint16_t flags; // Bit 0: Carry, Bit 2: Parity, Bit 4: Auxiliary Carry, Bit 6: Zero, Bit 7: Sign, Bit 8: Trap, Bit 9: Interrupt Enable, Bit 10: Direction, Bit 11: Overflow
    int halted;
} Cpu16;

static int cpu16_physical_address(
    uint16_t segment,
    uint16_t offset,
    uint32_t *physical_address)
{
    uint32_t address = ((uint32_t)segment << 4) + (uint32_t)offset;

    if (!physical_address || address >= CPU16_MEMORY_SIZE)
        return 0;

    *physical_address = address;
    return 1;
}

static int cpu16_fetch8(Cpu16 *cpu, uint8_t *value)
{
    uint32_t physical_address;

    if (!cpu || !value ||
        !cpu16_physical_address(cpu->cs, cpu->ip, &physical_address))
        return 0;

    if (physical_address < cpu->image_start ||
        (size_t)(physical_address - cpu->image_start) >= cpu->image_size)
        return 0;

    *value = cpu->memory[physical_address];
    cpu->ip = (uint16_t)(cpu->ip + 1u);
    return 1;
}

static int cpu16_push16(Cpu16 *cpu, uint16_t value)
{
    uint32_t address;
    uint16_t new_sp;

    if (!cpu)
        return 0;

    new_sp = (uint16_t)(cpu->sp - 2u);

    if (!cpu16_physical_address(cpu->ss, new_sp, &address) ||
        address > CPU16_MEMORY_SIZE - 2u)
        return 0;

    cpu->memory[address] = (uint8_t)(value & 0xFF);
    cpu->memory[address + 1] = (uint8_t)(value >> 8);
    cpu->sp = new_sp;
    return 1;
}

static int cpu16_pop16(Cpu16 *cpu, uint16_t *value)
{
    uint32_t address;

    if (!cpu || !value ||
        !cpu16_physical_address(cpu->ss, cpu->sp, &address) ||
        address > CPU16_MEMORY_SIZE - 2u)
        return 0;

    *value = (uint16_t)((uint16_t)cpu->memory[address] |
                        ((uint16_t)cpu->memory[address + 1] << 8));
    cpu->sp = (uint16_t)(cpu->sp + 2u);
    return 1;
}

static uint16_t *cpu16_reg16(Cpu16 *cpu, unsigned code)
{
    if (!cpu)
        return NULL;

    switch (code)
    {
    case 0:
        return &cpu->ax;
    case 1:
        return &cpu->cx;
    case 2:
        return &cpu->dx;
    case 3:
        return &cpu->bx;
    case 4:
        return &cpu->sp;
    case 5:
        return &cpu->bp;
    case 6:
        return &cpu->si;
    case 7:
        return &cpu->di;
    default:
        return NULL;
    }
}

static void cpu16_write_flag(Cpu16 *cpu, uint16_t flag, int set)
{
    if (set)
        cpu->flags |= flag;
    else
        cpu->flags &= (uint16_t)~flag;
}

static int even_parity8(uint8_t value)
{
    unsigned ones = 0;

    for (unsigned bit = 0; bit < 8; bit++)
        ones += (value >> bit) & 1u;

    return (ones & 1u) == 0;
}

static int emulate_instruction(Cpu16 *cpu)
{
    uint16_t instruction_ip;
    uint8_t opcode;

    if (!cpu || cpu->halted)
        return 0;

    instruction_ip = cpu->ip;

    if (!cpu16_fetch8(cpu, &opcode))
    {
        fprintf(stderr,
                "instruction fetch outside memory at %04X:%04X\n",
                (unsigned int)cpu->cs,
                (unsigned int)instruction_ip);
        return 0;
    }

    if (opcode >= 0x40u && opcode <= 0x47u)
    {
        unsigned code = opcode & 0x07u;
        uint16_t *reg = cpu16_reg16(cpu, code);
        uint16_t old_value;
        uint16_t result;

        if (!reg)
            return 0;

        old_value = *reg;
        result = (uint16_t)(old_value + 1u);
        *reg = result;

        cpu16_write_flag(cpu, CPU16_FLAG_OF, old_value == 0x7FFFu);
        cpu16_write_flag(cpu, CPU16_FLAG_SF,
                         (result & 0x8000u) != 0);
        cpu16_write_flag(cpu, CPU16_FLAG_ZF, result == 0);
        cpu16_write_flag(cpu, CPU16_FLAG_AF,
                         (old_value & 0x000Fu) == 0x000Fu);
        cpu16_write_flag(cpu, CPU16_FLAG_PF,
                         even_parity8((uint8_t)result));

        return 1;
    }
    if (opcode >= 0x48u && opcode <= 0x4Fu)
    {
        unsigned code = opcode & 0x07u;
        uint16_t *reg = cpu16_reg16(cpu, code);
        uint16_t old_value;
        uint16_t result;

        if (!reg)
            return 0;

        old_value = *reg;
        result = (uint16_t)(old_value - 1u);
        *reg = result;

        cpu16_write_flag(cpu, CPU16_FLAG_OF, old_value == 0x8000u);
        cpu16_write_flag(cpu, CPU16_FLAG_SF,
                         (result & 0x8000u) != 0);
        cpu16_write_flag(cpu, CPU16_FLAG_ZF, result == 0);
        cpu16_write_flag(cpu, CPU16_FLAG_AF,
                         (old_value & 0x000Fu) == 0);
        cpu16_write_flag(cpu, CPU16_FLAG_PF,
                         even_parity8((uint8_t)result));

        return 1;
    }
    if (opcode >= 0x50u && opcode <= 0x57u)
    {
        uint16_t *reg = cpu16_reg16(cpu, opcode & 0x07u);
        uint16_t value;

        if (!reg)
            return 0;

        value = *reg;
        return cpu16_push16(cpu, value);
    }
    if (opcode >= 0x58u && opcode <= 0x5Fu)
    {
        uint16_t *reg = cpu16_reg16(cpu, opcode & 0x07u);
        uint16_t value;

        if (!reg || !cpu16_pop16(cpu, &value))
            return 0;

        *reg = value;
        return 1;
    }
    if (opcode >= 0x90u && opcode <= 0x97u)
    {
        uint16_t *reg = cpu16_reg16(cpu, opcode & 0x07u);
        uint16_t temporary;

        if (!reg)
            return 0;

        temporary = cpu->ax;
        cpu->ax = *reg;
        *reg = temporary;
        return 1;
    }
    if (opcode >= 0xB8u && opcode <= 0xBFu)
    {
        uint16_t *reg = cpu16_reg16(cpu, opcode & 0x07u);
        uint8_t low;
        uint8_t high;

        if (!reg ||
            !cpu16_fetch8(cpu, &low) ||
            !cpu16_fetch8(cpu, &high))
        {
            fprintf(stderr,
                    "truncated MOV instruction at %04X:%04X\n",
                    (unsigned)cpu->cs,
                    (unsigned)instruction_ip);
            return 0;
        }

        *reg = (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
        return 1;
    }
    if (opcode == 0x8Eu)
    {
        uint8_t modrm;
        unsigned segment_code;
        uint16_t *source_register;
        uint16_t *destination_segment = NULL;

        if (!cpu16_fetch8(cpu, &modrm))
            return 0;

        if ((modrm & 0xC0u) != 0xC0u)
            return 0;

        source_register = cpu16_reg16(cpu, modrm & 0x07u);
        segment_code = (modrm >> 3) & 0x07u;

        if (segment_code == 0u)
            destination_segment = &cpu->es;
        else if (segment_code == 2u)
            destination_segment = &cpu->ss;
        else if (segment_code == 3u)
            destination_segment = &cpu->ds;

        if (!source_register || !destination_segment)
            return 0;

        *destination_segment = *source_register;
        return 1;
    }

    switch (opcode)
    {
    case 0xFD:                       /* STD */
        cpu->flags |= CPU16_FLAG_DF; // Set direction flag
        return 1;

    case 0xF8:                        /* CLC */
        cpu->flags &= ~CPU16_FLAG_CF; // Clear carry flag
        return 1;
    case 0xF9:                       /* STC */
        cpu->flags |= CPU16_FLAG_CF; // Set carry flag
        return 1;
    case 0xFA:                        /* CLI */
        cpu->flags &= ~CPU16_FLAG_IF; // Clear interrupt enable flag
        return 1;

    case 0xFB:                       /* STI */
        cpu->flags |= CPU16_FLAG_IF; // Set interrupt enable flag
        return 1;
    case 0xF5: /* CMC */
        // Complement carry flag (simulated as toggling interrupt_enabled for simplicity)
        cpu->flags ^= CPU16_FLAG_CF; // Complement carry flag
        return 1;
    case 0xFC: /* CLD */
        // Clear direction flag
        cpu->flags &= ~CPU16_FLAG_DF;
        return 1;

    case 0xF4: /* HLT */
        cpu->halted = 1;
        return 1;
    case 0x9F: /* LAHF */
    {
        uint16_t mask =
            CPU16_FLAG_CF | CPU16_FLAG_PF | CPU16_FLAG_AF |
            CPU16_FLAG_ZF | CPU16_FLAG_SF;

        uint8_t ah = (uint8_t)((cpu->flags & mask) | CPU16_FLAG_FIXED);
        cpu->ax = (uint16_t)((cpu->ax & 0x00FFu) | ((uint16_t)ah << 8));
        return 1;
    }

    case 0x9E: /* SAHF */
    {
        uint16_t mask =
            CPU16_FLAG_CF | CPU16_FLAG_PF | CPU16_FLAG_AF |
            CPU16_FLAG_ZF | CPU16_FLAG_SF;

        uint16_t ah = (uint16_t)(cpu->ax >> 8);
        cpu->flags = (uint16_t)((cpu->flags & ~mask) |
                                (ah & mask) |
                                CPU16_FLAG_FIXED);
        return 1;
    }
    case 0x9C: /* PUSHF */
        return cpu16_push16(cpu, cpu->flags | CPU16_FLAG_FIXED);

    case 0x9D: /* POPF */
    {
        uint16_t flags;

        if (!cpu16_pop16(cpu, &flags))
            return 0;

        cpu->flags = (uint16_t)(flags | CPU16_FLAG_FIXED);
        return 1;
    }
    case 0x98: /* CBW */
        cpu->ax = (uint16_t)((int8_t)(cpu->ax & 0x00FFu));
        return 1;
    case 0x99: /* CWD */
        cpu->dx = (cpu->ax & 0x8000u) ? 0xFFFFu : 0x0000u;
        return 1;
    case 0xCF: /* IRET */
        // Pop IP, CS, and FLAGS from the stack
        {
            uint16_t ip, cs, flags;

            if (!cpu16_pop16(cpu, &ip) ||
                !cpu16_pop16(cpu, &cs) ||
                !cpu16_pop16(cpu, &flags))
                return 0;

            cpu->ip = ip;
            cpu->cs = cs;
            cpu->flags = (uint16_t)(flags | CPU16_FLAG_FIXED);
            return 1;
        }

    case 0xCD: /* INT imm8: the KEMU BIOS shim below is intentionally small. */
    {
        uint8_t vector;

        if (!cpu16_fetch8(cpu, &vector))
        {
            fprintf(stderr,
                    "truncated INT instruction at %04X:%04X\n",
                    (unsigned int)cpu->cs,
                    (unsigned int)instruction_ip);
            return 0;
        }

        if (vector != 0x10u)
        {
            fprintf(stderr,
                    "unsupported BIOS interrupt %02Xh\n",
                    (unsigned int)vector);
            return 0;
        }

        if ((cpu->ax >> 8) != 0x0Eu)
        {
            fprintf(stderr,
                    "unsupported BIOS int 10h function AH=%02X\n",
                    (unsigned int)(cpu->ax >> 8));
            return 0;
        }

        if (putchar((unsigned char)(cpu->ax & 0x00FFu)) == EOF)
            return 0;

        return 1;
    }

    case 0x72: /* JC/JB rel8 */
    case 0x73: /* JNC/JAE rel8 */
    {
        uint8_t encoded_displacement;
        int carry_set;
        int take_branch;

        if (!cpu16_fetch8(cpu, &encoded_displacement))
        {
            fprintf(stderr,
                    "truncated conditional jump instruction at %04X:%04X\n",
                    (unsigned int)cpu->cs,
                    (unsigned int)instruction_ip);
            return 0;
        }

        carry_set = (cpu->flags & CPU16_FLAG_CF) != 0;
        take_branch = opcode == 0x72 ? carry_set : !carry_set;

        if (take_branch)
        {
            cpu->ip = (uint16_t)(cpu->ip + (int16_t)(int8_t)encoded_displacement);
        }

        return 1;
    }

    case 0xEB: /* JMP rel8 */
    {
        uint8_t encoded_displacement;

        if (!cpu16_fetch8(cpu, &encoded_displacement))
        {
            fprintf(stderr,
                    "truncated JMP instruction at %04X:%04X\n",
                    (unsigned int)cpu->cs,
                    (unsigned int)instruction_ip);
            return 0;
        }

        cpu->ip = (uint16_t)(cpu->ip + (int16_t)(int8_t)encoded_displacement);
        return 1;
    }
    case 0xEA: /* JMP ptr16:16 */
    {
        uint8_t offset_low;
        uint8_t offset_high;
        uint8_t segment_low;
        uint8_t segment_high;
        uint16_t target_offset;
        uint16_t target_segment;
        uint32_t target_address;

        if (!cpu16_fetch8(cpu, &offset_low) ||
            !cpu16_fetch8(cpu, &offset_high) ||
            !cpu16_fetch8(cpu, &segment_low) ||
            !cpu16_fetch8(cpu, &segment_high))
        {
            fprintf(stderr,
                    "truncated far JMP instruction at %04X:%04X\n",
                    (unsigned int)cpu->cs,
                    (unsigned int)instruction_ip);
            return 0;
        }

        target_offset = (uint16_t)((uint16_t)offset_low |
                                   ((uint16_t)offset_high << 8));
        target_segment = (uint16_t)((uint16_t)segment_low |
                                    ((uint16_t)segment_high << 8));

        if (!cpu16_physical_address(
                target_segment,
                target_offset,
                &target_address) ||
            target_address < cpu->image_start ||
            (size_t)(target_address - cpu->image_start) >= cpu->image_size)
        {
            fprintf(stderr,
                    "far JMP target outside loaded image at %04X:%04X\n",
                    (unsigned int)target_segment,
                    (unsigned int)target_offset);
            return 0;
        }

        cpu->ip = target_offset;
        cpu->cs = target_segment;
        return 1;
    }

    default:
        fprintf(stderr,
                "unknown opcode %02X at %04X:%04X\n",
                (unsigned int)opcode,
                (unsigned int)cpu->cs,
                (unsigned int)instruction_ip);
        return 0;
    }
}

int main(int argc, char **argv)
{
    uint32_t entry_address;

    if (!cpu16_physical_address(0x0000, 0x7C00, &entry_address))
    {
        fprintf(stderr, "invalid real-mode entry address\n");
        return 1;
    }
    // Initialize CPU and memory here before starting emulation
    static Cpu16 cpu = {0};
    cpu.cs = 0x0000;
    cpu.ds = 0x0000;
    cpu.es = 0x0000;
    cpu.ss = 0x0000;
    cpu.sp = 0x7C00;
    cpu.ip = 0x0000;
    cpu.flags = 0; // Clear all flags initially
    cpu.halted = 0;

    // Set the entry point in the instruction pointer
    cpu.ip = (uint16_t)(entry_address & 0xFFFF);

    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <program.bin>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file)
    {
        fprintf(stderr, "failed to open file: %s\n", argv[1]);
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "failed to measure file: %s\n", argv[1]);
        fclose(file);
        return 1;
    }

    long file_size = ftell(file);
    if (file_size < 0 ||
        (uint64_t)file_size > CPU16_MEMORY_SIZE - entry_address)
    {
        fprintf(stderr, "program does not fit in emulated memory\n");
        fclose(file);
        return 1;
    }

    if (fseek(file, 0, SEEK_SET) != 0 ||
        fread(&cpu.memory[entry_address], 1, (size_t)file_size, file) !=
            (size_t)file_size)
    {
        fprintf(stderr, "failed to read file: %s\n", argv[1]);
        fclose(file);
        return 1;
    }

    cpu.image_start = entry_address;
    cpu.image_size = (size_t)file_size;

    if (fclose(file) != 0)
    {
        fprintf(stderr, "failed to close file: %s\n", argv[1]);
        return 1;
    }

    // Start the emulation loop here
    const size_t max_steps = 1000;
    size_t steps = 0;

    while (!cpu.halted && steps < max_steps)
    {
        // Fetch and execute the next instruction
        if (!emulate_instruction(&cpu))
            return 1;
        steps++;
    }

    if (!cpu.halted)
    {
        fprintf(stderr, "step limit exceeded after %zu instructions\n",
                max_steps);
        return 1;
    }

    printf("halted after %zu instructions at %04X:%04X, AX=%04X\n",
           steps,
           (unsigned int)cpu.cs,
           (unsigned int)cpu.ip,
           (unsigned int)cpu.ax);

    return 0;
}

#include <stdio.h>
#include <stdint.h>

#define CPU16_MEMORY_SIZE (1024u * 1024u)

typedef struct {
    unsigned char memory[CPU16_MEMORY_SIZE];
    uint32_t image_start;
    size_t image_size;
    uint16_t ax, bx, cx, dx;
    uint16_t sp, bp, si, di;
    uint16_t cs, ds, es, ss, ip;
    int interrupt_enabled;
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

    switch (opcode)
    {
    case 0xB8: /* MOV AX, imm16 */
    {
        uint8_t low;
        uint8_t high;

        if (!cpu16_fetch8(cpu, &low) || !cpu16_fetch8(cpu, &high))
        {
            fprintf(stderr,
                    "truncated MOV instruction at %04X:%04X\n",
                    (unsigned int)cpu->cs,
                    (unsigned int)instruction_ip);
            return 0;
        }

        cpu->ax = (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
        return 1;
    }

    case 0xFA: /* CLI */
        cpu->interrupt_enabled = 0;
        return 1;

    case 0xFB: /* STI */
        cpu->interrupt_enabled = 1;
        return 1;

    case 0xF4: /* HLT */
        cpu->halted = 1;
        return 1;

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

        cpu->ip = (uint16_t)(
            cpu->ip + (int16_t)(int8_t)encoded_displacement);
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

int main(int argc, char **argv) {
    uint32_t entry_address;

    if (!cpu16_physical_address(0x0000, 0x7C00, &entry_address)) {
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
    cpu.interrupt_enabled = 1;
    cpu.halted = 0;

    // Set the entry point in the instruction pointer
    cpu.ip = (uint16_t)(entry_address & 0xFFFF);

    if(argc < 2) {
        fprintf(stderr, "usage: %s <program.bin>\n", argv[0]);
        return 1;
    }
    
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "failed to open file: %s\n", argv[1]);
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "failed to measure file: %s\n", argv[1]);
        fclose(file);
        return 1;
    }

    long file_size = ftell(file);
    if (file_size < 0 ||
        (uint64_t)file_size > CPU16_MEMORY_SIZE - entry_address) {
        fprintf(stderr, "program does not fit in emulated memory\n");
        fclose(file);
        return 1;
    }

    if (fseek(file, 0, SEEK_SET) != 0 ||
        fread(&cpu.memory[entry_address], 1, (size_t)file_size, file) !=
            (size_t)file_size) {
        fprintf(stderr, "failed to read file: %s\n", argv[1]);
        fclose(file);
        return 1;
    }

    cpu.image_start = entry_address;
    cpu.image_size = (size_t)file_size;

    if (fclose(file) != 0) {
        fprintf(stderr, "failed to close file: %s\n", argv[1]);
        return 1;
    }

    // Start the emulation loop here
    const size_t max_steps = 1000;
    size_t steps = 0;

    while (!cpu.halted && steps < max_steps) {
        // Fetch and execute the next instruction
        if (!emulate_instruction(&cpu))
            return 1;
        steps++;
    }

    if (!cpu.halted) {
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

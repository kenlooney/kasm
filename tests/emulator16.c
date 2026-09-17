// Copyright 2026 Kenneth Looney
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This emulator simulates a simple 16-bit CPU with opcodes that the assembler
// knows.

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint16_t ax, cx, dx, bx;
    uint16_t ip, sp;
    uint16_t flags;
    uint8_t memory[65536];

} Cpu16;

static int reset_cpu(Cpu16 *cpu, const unsigned char *image, size_t length, uint16_t load_address){
    if (length > sizeof(cpu->memory) - load_address)
        return 0;
    memset(cpu, 0, sizeof(*cpu));
    memcpy(cpu->memory + load_address, image, length);
    cpu->ip = load_address;
    cpu->sp = 0xFFFE; // Initialize stack pointer to top of memory (typical for 16-bit real mode)
    cpu->flags = 0x0002; // Initialize flags register with the reserved bit set (typical for 16-bit real mode)
    cpu->ax = cpu->cx = cpu->dx = cpu->bx = 0; // Initialize general-purpose registers to 0
    return 1;
}
static int cpu16_step(Cpu16 *cpu){
    uint8_t *p = cpu->memory + cpu->ip;
    // Decode and execute instruction at current IP
    
    // Handle MOV immediate to register instructions (B8-BF)
    if(p[0] == 0xB8) { cpu->ax = (uint16_t)(p[1] | (p[2] << 8)); cpu->ip += 3; return 1;}
    if(p[0] == 0xB9) { cpu->cx = (uint16_t)(p[1] | (p[2] << 8)); cpu->ip += 3; return 1;}
    if(p[0] == 0xBA) { cpu->dx = (uint16_t)(p[1] | (p[2] << 8)); cpu->ip += 3; return 1;}
    if(p[0] == 0xBB) { cpu->bx = (uint16_t)(p[1] | (p[2] << 8)); cpu->ip += 3; return 1;}
    // Handle ADD immediate to AX instruction (05)
    if(p[0] == 0x05) { cpu->ax = (uint16_t)(cpu->ax + (p[1] | (p[2] << 8))); cpu->ip += 3; return 1;}
    // Handle SUB immediate from AX instruction (2D)
    if(p[0] == 0x2D) { cpu->ax = (uint16_t)(cpu->ax - (p[1] | (p[2] << 8))); cpu->ip += 3; return 1;}
    // Handle ADD/SUB immediate to the other 16-bit registers (81 /r)
    if (p[0] == 0x81 && (p[1] == 0xC1 || p[1] == 0xC2 ||
                         p[1] == 0xE9 || p[1] == 0xEA))
    {
        uint16_t value = (uint16_t)(p[2] | (p[3] << 8));
        uint16_t *register_value = p[1] == 0xC1 || p[1] == 0xE9
                                        ? &cpu->cx
                                        : &cpu->dx;
        if (p[1] == 0xC1 || p[1] == 0xC2)
            *register_value = (uint16_t)(*register_value + value);
        else
            *register_value = (uint16_t)(*register_value - value);
        cpu->ip += 4;
        return 1;
    }
    // Handle MOV from memory at [BX] to register (8B /r with ModR/M byte 07)
    if(p[0] == 0x8B && (p[1] & 0xC7) == 0x07)
    {
        uint16_t value = (uint16_t)(cpu->memory[cpu->bx] | (cpu->memory[cpu->bx + 1] << 8));
        unsigned reg = (p[1] >> 3) & 0x07;
        switch(reg) {
            case 0: cpu->ax = value; break;
            case 1: cpu->cx = value; break;
            case 2: cpu->dx = value; break;
            case 3: cpu->bx = value; break;
        }
        cpu->ip += 2; // Advance past the instruction and ModR/M byte
        return 1;
    }
    return 0; // Instruction not recognized
}

static int cpu_run(Cpu16 *cpu, int instruction_limit)
{
    int steps = 0;
    while (steps < instruction_limit && cpu16_step(cpu))
    {
        steps++;
        printf("step %d: IP=%04X AX=%04X CX=%04X DX=%04X BX=%04X\n",
               steps, cpu->ip, cpu->ax, cpu->cx, cpu->dx, cpu->bx);
    }
    return steps;
}

static int run_boot_image(const char *path)
{
    unsigned char image[sizeof(((Cpu16 *)0)->memory)];
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open boot image: %s\n", path);
        return 1;
    }
    size_t length = fread(image, 1, sizeof(image), file);
    int read_error = ferror(file);
    int has_more = fgetc(file) != EOF;
    fclose(file);
    if (read_error || has_more || length != 512)
    {
        fprintf(stderr, "Boot image must be exactly 512 bytes\n");
        return 1;
    }

    Cpu16 cpu;
    if (!reset_cpu(&cpu, image, length, 0x7C00))
        return 1;
    printf("boot image: %s loaded at 7C00\n", path);
    if (cpu_run(&cpu, 2) != 2 || cpu.ax != 45)
    {
        fprintf(stderr, "Boot image execution failed: AX=%04X\n", cpu.ax);
        return 1;
    }
    printf("boot image: AX=%04X at IP=%04X\n", cpu.ax, cpu.ip);
    return 0;
}

int main(int argc, char **argv)
{
    static const unsigned char mov_image[] = {
        0xB8, 0x2A, 0x00, 0xB9, 0x07, 0x00, 0xBA, 0x03, 0x00
    };
    static const unsigned char add_sub_image[] = {
        0x05, 0x07, 0x00, 0x81, 0xE9, 0x03, 0x00,
        0x81, 0xC2, 0x07, 0x00
    };
    static const unsigned char mov_indirect_image[] = {0x8B, 0x07};
    static const unsigned char bad_opcode[] = {0x90};
    static const unsigned char bad_modrm[] = {0x8B, 0x06};
    Cpu16 cpu;

    if (argc == 2)
        return run_boot_image(argv[1]);
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s [boot-image]\n", argv[0]);
        return 1;
    }

    if (!reset_cpu(&cpu, mov_image, sizeof(mov_image), 0))
        return 1;
    if (cpu_run(&cpu, 3) != 3 ||
        cpu.ax != 42 || cpu.cx != 7 || cpu.dx != 3)
        return 1;
    puts("mode16_mov: PASS");

    if (!reset_cpu(&cpu, add_sub_image, sizeof(add_sub_image), 0))
        return 1;
    cpu.cx = 10;
    if (cpu_run(&cpu, 3) != 3 ||
        cpu.ax != 7 || cpu.cx != 7 || cpu.dx != 7)
        return 1;
    puts("mode16_add_sub: PASS");

    if (!reset_cpu(&cpu, mov_indirect_image, sizeof(mov_indirect_image), 0))
        return 1;
    cpu.bx = 0x0100;
    cpu.memory[0x0100] = 0x2A;
    cpu.memory[0x0101] = 0x00;
    if (cpu_run(&cpu, 1) != 1 || cpu.ax != 42)
        return 1;
    puts("mode16_mov_indirect: PASS");

    if (!reset_cpu(&cpu, bad_opcode, sizeof(bad_opcode), 0))
        return 1;
    if (cpu16_step(&cpu))
        return 1;
    if (!reset_cpu(&cpu, bad_modrm, sizeof(bad_modrm), 0))
        return 1;
    if (cpu16_step(&cpu))
        return 1;
    puts("corrupted encodings: PASS");

    return 0;
}
#include "decode.h"
#include <stdint.h>
#include <stdio.h>
static uint64_t read_le(const unsigned char *data, int count)
{
    uint64_t value = 0;
    for (int i = 0; i < count; i++)
        value |= (uint64_t)data[i] << (8 * i);
    return value;
}
static long long signed_displacement(uint64_t value, int bits)
{
    uint64_t sign = (uint64_t)1 << (bits - 1);
    if (value < sign)
        return (long long)value;
    return (long long)value - (long long)((uint64_t)1 << bits);
}
int decode(const Bytes *bytes, const Target *target)
{
    size_t at = 0;
    while (at < bytes->count)
    {
        const unsigned char *p = bytes->data + at;
        size_t left = bytes->count - at;
        size_t width = 0;
        printf("%04zu: ", at);
        int immediate_width = target->mode == MODE_16 ? 2 : 4;
        const char *registers16[] = {"ax", "cx", "dx", "bx"};
        const char *registers32[] = {"eax", "ecx", "edx", "ebx"};
        const char **registers = target->mode == MODE_16 ? registers16 : registers32;
        
        if (p[0] == 0x8B && left >= 2 && (p[1] & 0xC7) == 0x07)
        {
            unsigned reg = (p[1] >> 3) & 0x07;
            const char *name = reg == 0 ? "ax" : reg == 1 ? "cx" : reg == 2 ? "dx" : NULL;
            if (name == NULL)
                return 0;
            printf("mov %s, [bx]\n", name);
            width = 2;
        }
        else if (p[0] >= 0xB8 && p[0] <= 0xBA && left >= (size_t)(1 + immediate_width))
        {
            printf("mov %s, %llu\n", registers[p[0] - 0xB8],
                   (unsigned long long)read_le(p + 1, immediate_width));
            width = 1 + immediate_width;
        }
        else if ((p[0] == 0x05 || p[0] == 0x2D) &&
                 left >= (size_t)(1 + immediate_width))
        {
            printf("%s %s, %lld\n", p[0] == 0x05 ? "add" : "sub", registers[0],
                   signed_displacement(read_le(p + 1, immediate_width), immediate_width * 8));
            width = 1 + immediate_width;
        }
        else if (p[0] == 0x81 && left >= (size_t)(2 + immediate_width) &&
                 ((p[1] >= 0xC1 && p[1] <= 0xC3) ||
                  (p[1] >= 0xE9 && p[1] <= 0xEB)))
        {
            printf("%s %s, %lld\n", p[1] < 0xE0 ? "add" : "sub", registers[p[1] & 7],
                   signed_displacement(read_le(p + 2, immediate_width), immediate_width * 8));
            width = 2 + immediate_width;
        }
        else if (p[0] == 0xC3)
        {
            puts("ret");
            width = 1;
        }
        else if (p[0] == 0x50)
        {
            puts("push rax");
            width = 1;
        }
        else if (p[0] == 0x58)
        {
            puts("pop rax");
            width = 1;
        }
        else if (p[0] == 0x0D && left >= 5)
        {
            printf("or eax, %lld\n",
                   signed_displacement(read_le(p + 1, 4), 32));
            width = 5;
        }
        // xor rim
        else if (p[0] == 0x35 && left >= 5)
        {
            printf("xor eax, %lld\n", signed_displacement(read_le(p + 1, 4), 32));
            width = 5;
        }
        // and rim
        else if (p[0] == 0x25 && left >= 5)
        {
            printf("and eax, %lld\n", signed_displacement(read_le(p + 1, 4), 32));
            width = 5;
        }
        else if (p[0] == 0x3D && left >= 5)
        {
            printf("cmp eax, %lld\n", signed_displacement(read_le(p + 1, 4), 32));
            width = 5;
        }
        else if (p[0] == 0x15 && left >= 5)
        {
            printf("adc eax, %lld\n", signed_displacement(read_le(p + 1, 4), 32));
            width = 5;
        }
        else if (p[0] == 0xCD && left >= 2)
        {
            printf("int 0x%02X\n", (unsigned int)p[1]);
            width = 2;
        }
        else if (p[0] == 0xFF && left >= 2 && p[1] == 0xC8)
        {
            puts("dec eax");
            width = 2;
        }
        else if (p[0] == 0xFF && left >= 2 && p[1] == 0xC0)
        {
            puts("inc eax");
            width = 2;
        }
        else if (p[0] == 0x0F && left >= 6 && p[1] == 0x84)
        {
            width = 6;
            long long d = signed_displacement(read_le(p + 2, 4), 32);
            printf("jz target=%lld\n", (long long)(at + width) + d);
        }
        else if ((p[0] == 0xE9 && left >= 5) || (p[0] == 0xEB && left >= 2))
        {
            width = p[0] == 0xEB ? 2 : 5;
            int bits = width == 2 ? 8 : 32;
            long long d = signed_displacement(read_le(p + 1, (int)width - 1), bits);
            printf("jmp target=%lld\n", (long long)(at + width) + d);
        }
        else if (p[0] == 0x0F && left >= 6 && p[1] == 0x85)
        {
            width = 6;
            long long d = signed_displacement(read_le(p + 2, 4), 32);
            printf("jnz target=%lld\n", (long long)(at + width) + d);
        }
        else if (p[0] == 0x0F && left >= 6 && p[1] == 0x82)
        {
            width = 6;
            long long d = signed_displacement(read_le(p + 2, 4), 32);
            printf("jb target=%lld\n", (long long)(at + width) + d);
        }
        else if (p[0] == 0x0F && left >= 6 && p[1] == 0x8C)
        {
            width = 6;
            long long d = signed_displacement(read_le(p + 2, 4), 32);
            printf("jl target=%lld\n", (long long)(at + width) + d);
        }
        else if (p[0] == 0xFF && left >= 14 && p[1] == 0x25 && read_le(p + 2, 4) == 0)
        {
            printf("jmpabs image-offset=%llu (address slot follows)\n",
                   (unsigned long long)read_le(p + 6, 8));
            width = 14;
        }
        else
        {
            puts("unknown or truncated encoding");
            return 0;
        }
        at += width;
    }
    return 1;
}

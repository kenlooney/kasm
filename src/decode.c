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
int decode(const Bytes *bytes)
{
    size_t at = 0;
    while (at < bytes->count)
    {
        const unsigned char *p = bytes->data + at;
        size_t left = bytes->count - at;
        size_t width = 0;
        printf("%04zu: ", at);
        if (p[0] == 0xB8 && left >= 5)
        {
            printf("mov eax, %llu\n", (unsigned long long)read_le(p + 1, 4));
            width = 5;
        }
        else if (p[0] == 0xB9 && left >= 5)
        {
            printf("mov ecx, %llu\n", (unsigned long long)read_le(p + 1, 4));
            width = 5;
        }
        else if (p[0] == 0xBA && left >= 5)
        {
            printf("mov edx, %llu\n", (unsigned long long)read_le(p + 1, 4));
            width = 5;
        }
        else if (p[0] == 0x05 && left >= 5)
        {
            printf("add eax, %lld\n", signed_displacement(read_le(p + 1, 4), 32));
            width = 5;
        }
        else if (p[0] == 0x81 && left >= 6 && p[1] == 0xC1)
        {
            printf("add ecx, %lld\n", signed_displacement(read_le(p + 2, 4), 32));
            width = 6;
        }
        else if (p[0] == 0x81 && left >= 6 && p[1] == 0xC2)
        {
            printf("add edx, %lld\n", signed_displacement(read_le(p + 2, 4), 32));
            width = 6;
        }
        else if (p[0] == 0x2D && left >= 5)
        {
            printf("sub eax, %lld\n", signed_displacement(read_le(p + 1, 4), 32));
            width = 5;
        }
        else if (p[0] == 0x81 && left >= 6 && p[1] == 0xE9)
        {
            printf("sub ecx, %lld\n", signed_displacement(read_le(p + 2, 4), 32));
            width = 6;
        }
        else if (p[0] == 0x81 && left >= 6 && p[1] == 0xEA)
        {
            printf("sub edx, %lld\n", signed_displacement(read_le(p + 2, 4), 32));
            width = 6;
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

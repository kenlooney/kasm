#include "kasm/emit.h"
#include "kasm/encode.h"
#include "kasm/layout.h"
#include "kasm/program.h"
#include "kasm/target.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    static const uint8_t expected[] = {
        0x40, 0x41, 0x42, 0x43,
        0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4A, 0x4B,
        0x4C, 0x4D, 0x4E, 0x4F,
        0x50, 0x51, 0x52, 0x53,
        0x54, 0x55, 0x56, 0x57,
        0x5F, 0x5E, 0x5D, 0x5C,
        0x5B, 0x5A, 0x59, 0x58,
        0xB8, 0x01, 0x00,
        0xBF, 0x34, 0x12,
        0x90, 0x91, 0x92, 0x93,
        0x94, 0x95, 0x96, 0x97,
        0xF4, 0xEB, 0xFD
    };
    const Target target = {
        .arch = ARCH_X86,
        .mode = MODE_16
    };
    Source source = {0};
    Parser parser = {0};
    Program program = {0};
    Bytes bytes = {0};
    int result = 1;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s input.asm\n", argv[0]);
        return 1;
    }

    if (source_load(&source, argv[1]) != 0)
        goto cleanup;

    parser_start(&parser, &source);

    if (!parse_program(&parser, &program) ||
        !evaluate_program(&parser, &program) ||
        !layout_program(&program, &target) ||
        !check_program(&parser, &program, &target) ||
        !emit_program(&program, &target, &bytes))
    {
        goto cleanup;
    }

    if (bytes.count != sizeof expected)
    {
        fprintf(stderr,
                "expected %zu bytes, got %zu\n",
                sizeof expected,
                bytes.count);
        goto cleanup;
    }

    for (size_t i = 0; i < sizeof expected; i++)
    {
        if (bytes.data[i] != expected[i])
        {
            fprintf(stderr,
                    "byte %zu: expected %02X, got %02X\n",
                    i,
                    (unsigned int)expected[i],
                    (unsigned int)bytes.data[i]);
            goto cleanup;
        }
    }

    for (size_t i = 0; i < bytes.count; i++)
    {
        printf("%s%02X",
               i == 0 ? "" : " ",
               (unsigned int)bytes.data[i]);
    }
    putchar('\n');

    result = 0;

cleanup:
    bytes_free(&bytes);
    free(program.statements);
    free(parser.nodes);
    free((void *)source.text);
    return result;
}

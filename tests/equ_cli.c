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
    static const uint8_t expected_bytes[] = {
        0x00,
        0x01, 0x00,
        0x00, 0x7C
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

    if (!parse_program(&parser, &program, &target))
        goto cleanup;
    if (!evaluate_program(&parser, &program))
        goto cleanup;
    if (!layout_program(&program, &target))
        goto cleanup;
    if (!check_program(&parser, &program, &target))
        goto cleanup;
    if (!emit_program(&program, &target, &bytes))
        goto cleanup;

    if (bytes.count != sizeof expected_bytes)
    {
        fprintf(stderr,
                "expected %zu bytes, got %zu\n",
                sizeof expected_bytes,
                bytes.count);
        goto cleanup;
    }

    for (size_t i = 0; i < sizeof expected_bytes; i++)
    {
        if (bytes.data[i] != expected_bytes[i])
        {
            fprintf(stderr,
                    "offset %zu: expected %02X, got %02X\n",
                    i,
                    (unsigned int)expected_bytes[i],
                    (unsigned int)bytes.data[i]);
            goto cleanup;
        }
    }

    if (program.count != 8 ||
        program.statements[4].kind != ST_EQU ||
        program.statements[5].kind != ST_EQU ||
        program.statements[4].offset != 1 ||
        program.statements[5].offset != 1)
    {
        fprintf(stderr, "expected both equ statements at offset 1\n");
        goto cleanup;
    }

    result = 0;

cleanup:
    bytes_free(&bytes);
    free(program.statements);
    free(parser.nodes);
    free((void *)source.text);
    return result;
}
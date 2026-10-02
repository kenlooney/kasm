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
        0xB8, 0x11, 0x11,
        0xF8,
        0x72, 0x0E,
        0x73, 0x03,
        0xB8, 0xAD, 0xDE,
        0xF9,
        0x73, 0x0A,
        0x72, 0x03,
        0xB8, 0xEF, 0xBE,
        0xF4,
        0xB8, 0xAA, 0xAA,
        0xF4,
        0xB8, 0xBB, 0xBB,
        0xF4
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

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s input.asm output.bin\n", argv[0]);
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
                    "offset %zu: expected %02X, got %02X\n",
                    i,
                    (unsigned int)expected[i],
                    (unsigned int)bytes.data[i]);
            goto cleanup;
        }
    }

    FILE *output = fopen(argv[2], "wb");
    if (!output)
    {
        fprintf(stderr, "failed to open output file: %s\n", argv[2]);
        goto cleanup;
    }

    if (fwrite(bytes.data, 1, bytes.count, output) != bytes.count)
    {
        fprintf(stderr, "failed to write output file: %s\n", argv[2]);
        fclose(output);
        goto cleanup;
    }

    if (fclose(output) != 0)
    {
        fprintf(stderr, "failed to close output file: %s\n", argv[2]);
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

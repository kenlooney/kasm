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
    static const MachineMode expected_modes[] = {
        MODE_16,
        MODE_16,
        MODE_32,
        MODE_32,
        MODE_16,
        MODE_16
    };
    static const size_t expected_offsets[] = {
        0, 0, 3, 3, 8, 8
    };
    static const uint8_t expected_bytes[] = {
        0xB8, 0x34, 0x12,
        0xB8, 0x78, 0x56, 0x34, 0x12,
        0xB9, 0x78, 0x56
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
    {
        fprintf(stderr, "mode-region parsing failed\n");
        goto cleanup;
    }
    if (!evaluate_program(&parser, &program))
    {
        fprintf(stderr, "mode-region expression evaluation failed\n");
        goto cleanup;
    }
    if (!layout_program(&program, &target))
    {
        fprintf(stderr, "mode-region layout failed\n");
        goto cleanup;
    }
    if (!check_program(&parser, &program, &target))
    {
        fprintf(stderr, "mode-region semantic checking failed\n");
        goto cleanup;
    }
    if (!emit_program(&program, &target, &bytes))
    {
        fprintf(stderr, "mode-region encoding failed\n");
        goto cleanup;
    }

    if (program.count != 6)
    {
        fprintf(stderr,
                "expected 6 statements, got %d\n",
                program.count);
        goto cleanup;
    }

    for (int i = 0; i < program.count; i++)
    {
        if (program.statements[i].mode != expected_modes[i])
        {
            fprintf(stderr,
                    "statement %d: expected mode %d, got %d\n",
                    i,
                    (int)expected_modes[i],
                    (int)program.statements[i].mode);
            goto cleanup;
        }

        if (program.statements[i].offset != expected_offsets[i])
        {
            fprintf(stderr,
                    "statement %d: expected offset %zu, got %zu\n",
                    i,
                    expected_offsets[i],
                    program.statements[i].offset);
            goto cleanup;
        }
    }

    if (bytes.count != sizeof(expected_bytes))
    {
        fprintf(stderr,
                "expected %zu encoded bytes, got %zu\n",
                sizeof(expected_bytes),
                bytes.count);
        goto cleanup;
    }

    for (size_t i = 0; i < sizeof(expected_bytes); i++)
    {
        if (bytes.data[i] != expected_bytes[i])
        {
            fprintf(stderr,
                    "byte %zu: expected %02X, got %02X\n",
                    i,
                    (unsigned int)expected_bytes[i],
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

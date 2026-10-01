#include "kasm/emit.h"
#include "kasm/encode.h"
#include "kasm/program.h"
#include "kasm/target.h"
#include "kasm/layout.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s input.asm output.bin\n", argv[0]);
        return 1;
    }

    Source source = {0};
    Parser parser = {0};
    Program program = {0};
    Bytes bytes = {0};
    int result = 1;

    const Target target = {
        .arch = ARCH_X86,
        .mode = MODE_16};

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

    if (!program.has_origin || program.origin != 0x7C00)
    {
        fprintf(stderr, "expected origin 0x7C00\n");
        goto cleanup;
    }

    if (!emit_program(&program, &target, &bytes))
        goto cleanup;

    if (bytes.count != 512)
    {
        fprintf(stderr,
                "expected a 512-byte boot sector, got %zu bytes\n",
                bytes.count);
        goto cleanup;
    }

    if (
        bytes.data[0] != 0x99 ||
        bytes.data[1] != 0xCF
    )
    {
        fprintf(stderr, "expected boot code 99 CF\n");
        goto cleanup;
    }

    for (size_t i = 2; i < 510; i++)
    {
        if (bytes.data[i] != 0)
        {
            fprintf(stderr,
                    "expected zero padding at offset %zu, got %02X\n",
                    i,
                    (unsigned int)bytes.data[i]);
            goto cleanup;
        }
    }

    if (bytes.data[510] != 0x55 || bytes.data[511] != 0xAA)
    {
        fprintf(stderr, "expected boot signature 55 AA\n");
        goto cleanup;
    }

    printf("boot sector: %zu bytes, signature %02X %02X\n",
           bytes.count,
           (unsigned int)bytes.data[510],
           (unsigned int)bytes.data[511]);

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

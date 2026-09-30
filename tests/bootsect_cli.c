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

    if (!parse_program(&parser, &program))
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
        bytes.data[0] != 0xFD ||
        bytes.data[1] != 0xFC ||
        bytes.data[2] != 0xF5 ||
        bytes.data[3] != 0xF8 ||
        bytes.data[4] != 0x90 ||
        bytes.data[5] != 0xFA ||
        bytes.data[6] != 0x9F ||
        bytes.data[7] != 0x9E ||
        bytes.data[8] != 0x9C ||
        bytes.data[9] != 0x9D ||
        bytes.data[10] != 0xF4 ||
        bytes.data[11] != 0xEB ||
        bytes.data[12] != 0xFD
    )
    {
        fprintf(stderr, "expected boot code FD FC F5 F8 90 FA 9F 9E 9C 9D F4 EB FD\n");
        goto cleanup;
    }

    for (size_t i = 13; i < 510; i++)
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

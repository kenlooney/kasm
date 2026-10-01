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

    if (!emit_program(&program, &target, &bytes))
        goto cleanup;

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

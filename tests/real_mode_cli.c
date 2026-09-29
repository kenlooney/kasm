#include "kasm/emit.h"
#include "kasm/encode.h"
#include "kasm/program.h"
#include "kasm/target.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s input.asm\n", argv[0]);
        return 1;
    }

    Source source = {0};
    Parser parser = {0};
    Program program = {0};
    Bytes bytes = {0};
    int result = 1;

    const Target target = {
        .arch = ARCH_X86,
        .mode = MODE_16
    };

    if (source_load(&source, argv[1]) != 0)
        goto cleanup;

    parser_start(&parser, &source);

    if (!parse_program(&parser, &program))
        goto cleanup;

    if (!check_program(&parser, &program, &target))
        goto cleanup;

    if (!emit_program(&program, &target, &bytes))
        goto cleanup;

    if (bytes.count != 3 ||
        bytes.data[0] != 0xB8 ||
        bytes.data[1] != 0x34 ||
        bytes.data[2] != 0x12) {
        fprintf(stderr, "expected B8 34 12\n");
        goto cleanup;
    }

    for (size_t i = 0; i < bytes.count; i++) {
        printf(
            "%s%02X",
            i == 0 ? "" : " ",
            (unsigned int)bytes.data[i]
        );
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
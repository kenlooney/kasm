#include "kasm/program.h"
#include <stdio.h>
#include <stdlib.h>
#include "kasm/emit.h"
#include "kasm/target.h"

int main(int argc, char **argv) {
    // For now, we assume the target is 16-bit x86.
    const Target target = { .arch = ARCH_X86, .mode = MODE_16 };

    Source source;
    if (argc != 2) {
        fprintf(stderr, "usage: input.asm\n");
        return 1;
    }
    if (source_load(&source, argv[1]) != 0)
        return 1;

    Parser parser;
    Program program;
    parser_start(&parser, &source);
    if (!parse_program(&parser, &program)) {
        free(program.statements);
        free(parser.nodes);
        return 1;
    }
    if (!check_program(&parser, &program, &target)) {
        free(program.statements);
        free(parser.nodes);
        return 1;
    }
    Bytes bytes = {0};
    if (!byte_push(&bytes, 0x2A)) {
        free(program.statements);
        free(parser.nodes);
        return 1;
    }

    // Push enough bytes to exercise automatic capacity growth.
    for (size_t i = 0; i < KASM_BYTES_INITIAL_CAPACITY * 2; i++) {
        if (!byte_push(&bytes, (uint8_t)i)) {
            bytes_free(&bytes);
            free(program.statements);
            free(parser.nodes);
            return 1;
        }
    }

    printf("%02X\n", (unsigned int)bytes.data[0]);
    bytes_free(&bytes);
    free(program.statements);
    free(parser.nodes);
    return 0;
}

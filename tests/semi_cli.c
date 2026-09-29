#include "kasm/program.h"
#include <stdio.h>
#include <stdlib.h>
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
    if (!evaluate_program(&parser, &program)) {
        free(program.statements);
        free(parser.nodes);
        return 1;
    }
    if (!check_program(&parser, &program, &target)) {
        free(program.statements);
        free(parser.nodes);
        return 1;
    }
    printf("statements = %d\n", program.count);
    for (int i = 0; i < program.count; i++)
        printf("value = %lld\n", program.statements[i].value);
    free(program.statements);
    free(parser.nodes);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "kasm/program.h"
int main(int argc, char **argv) {
    Source source;
    if (argc != 2) {
        fprintf(stderr, "usage: lesson input.asm\n");
        return 1;
    }
    if (kasm_source_load(&source, argv[1]) != 0)
        return 1;

    Parser parser;
    Program program;
    parser_start(&parser, &source);
    if (!parse_program(&parser, &program)) {
        free(program.statements);
        free(parser.nodes);
        return 1;
    }
    printf("statements = %d\n", program.count);
    free(program.statements);
    free(parser.nodes);
    return 0;
}

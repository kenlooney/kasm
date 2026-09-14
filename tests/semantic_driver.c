// Copyright 2026 Kenneth Looney
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "semantic.h"

// Keep evaluated-value checks independent of the byte-output CLI.
int main(int argc, char **argv) {
    Source source;
    if (argc != 2 || !source_load(&source, argv[1]))
        return 1;
    Parser parser;
    Program program;
    parser_start(&parser, &source);
    int ok = parse_program(&parser, &program) && check_program(&parser, &program);
    if (ok) {
        printf("statements = %d\n", program.count);
        for (int i = 0; i < program.count; i++)
            printf("value = %lld\n", program.statements[i].value);
    }
    free(program.statements);
    free(parser.nodes);
    return ok ? 0 : 1;
}

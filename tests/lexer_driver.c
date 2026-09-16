// Copyright 2026 Kenneth Looney
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include "lexer.h"

// Keep lexer regression output independent of the assembler's parser CLI.
int main(int argc, char **argv) {
    Source source;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }
    if (!source_load(&source, argv[1]))
        return 1;

    Lexer lexer;
    lexer_start(&lexer, &source);
    while (!lexer.failed && lexer.token.kind != TK_END) {
        Token token = lexer.token;
        printf("token %d [%zu,%zu) value=%lld\n", (int)token.kind,
               token.span.start, token.span.end, token.value);
        lexer_next(&lexer);
    }
    int result = lexer.failed ? 1 : 0;
    source_free(&source);
    return result;
}

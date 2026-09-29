#include "kasm/diagnostic.h"
#include "kasm/source.h"
#include "kasm/lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv) {
    Source source;
    if (argc != 2) {
        fprintf(stderr, "usage: lesson input.asm\n");
        return 1;
    }
    if (source_load(&source, argv[1]) != 0)
        return 1;

    Lexer lexer;
    lexer_start(&lexer, &source);
    while (!lexer.failed && lexer.token.kind != TK_END) {
        Token t = lexer.token;
        printf("token %d [%zu,%zu) value=%lld\n", (int)t.kind, t.span.start, t.span.end, t.value);
        lexer_next(&lexer);
    }
    return lexer.failed ? 1 : 0;
}

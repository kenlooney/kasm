#include "kasm/expr.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    Source source;
    if (argc != 2) {
        fprintf(stderr, "usage: lesson input.asm\n");
        return 1;
    }
    if (kasm_source_load(&source, argv[1]) != 0)
        return 1;

    Parser parser;
    parser_start(&parser, &source);
    int root = parse_expression(&parser);
    if (root >= 0 && parser.lexer.token.kind != TK_END)
        parser_error(&parser, "expected end of input");
    if (root < 0 || parser.failed || parser.lexer.failed) {
        free(parser.nodes);
        return 1;
    }
    for (int i = 0; i < parser.count; i++) {
        Expr e = parser.nodes[i];
        printf("node %d: kind=%d value=%lld left=%d right=%d\n", i, (int)e.kind, e.value, e.left,
               e.right);
    }
    printf("root = %d\n", root);
    free(parser.nodes);
    return 0;
}

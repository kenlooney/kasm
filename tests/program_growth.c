// Copyright 2026 Kenneth Looney
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "program.h"

int main(void) {
    Source source = {0};
    source.path = "program-growth.asm";
    for (int i = 0; i < 300; i++)
        source.length += (size_t)sprintf(source.text + source.length, "mov eax,%d;", i);

    Parser parser;
    parser_start(&parser, &source);
    Program program;
    int ok = parse_program(&parser, &program);
    ok = ok && program.count == 300 && program.capacity >= program.count;
    for (int i = 0; ok && i < program.count; i++) {
        Statement s = program.statements[i];
        ok = s.kind == ST_MOV && token_is(&source, s.operand, "eax") &&
             s.expression >= 0 && s.expression < parser.count;
        if (ok) {
            Expr e = parser.nodes[s.expression];
            ok = e.kind == EX_INT && e.value == i;
        }
    }
    free(program.statements);
    free(parser.nodes);
    if (!ok)
        fprintf(stderr, "Expected 300 MOV statements with their original operands intact\n");
    return ok ? 0 : 1;
}

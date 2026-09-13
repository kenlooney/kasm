// Copyright 2026 Kenneth Looney
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "program.h"

int main(int argc, char **argv) {
    if (argc != 2)
        return 2;
    Source source = {0};
    source.path = "blocks.asm";
    int expected_count = 0;
    int expected_success = 1;
    int ordered = 0;
    if (strcmp(argv[1], "empty") == 0) {
        strcpy(source.text, "{} { {} }\n{\n\n}\n");
    } else if (strcmp(argv[1], "order") == 0) {
        strcpy(source.text,
               "mov eax, 1;\n{\nmov ebx, 0b10;\n{mov ecx, 3;}\n}\n"
               "{mov edx, 4;}\nmov esi, 5;");
        expected_count = 5;
        ordered = 1;
    } else if (strcmp(argv[1], "missing") == 0) {
        strcpy(source.text, "{mov eax, 1;");
        expected_success = 0;
    } else if (strcmp(argv[1], "extra") == 0) {
        strcpy(source.text, "{mov eax, 1;}} ");
        expected_success = 0;
    } else if (strcmp(argv[1], "limit") == 0 || strcmp(argv[1], "too_deep") == 0) {
        expected_success = strcmp(argv[1], "limit") == 0;
        size_t depth = (size_t)MAX_BLOCK_DEPTH + !expected_success;
        const char *instruction = "mov eax, 7;";
        if (depth > (sizeof(source.text) - strlen(instruction) - 1) / 2) {
            fprintf(stderr, "Depth test exceeds Source storage\n");
            return 2;
        }
        memset(source.text, '{', depth);
        strcpy(source.text + depth, instruction);
        memset(source.text + depth + strlen(instruction), '}', depth);
        expected_count = 1;
    } else {
        return 2;
    }
    source.length = strlen(source.text);
    Parser parser;
    Program program;
    parser_start(&parser, &source);
    int parsed = parse_program(&parser, &program);
    int ok = parsed == expected_success;
    if (expected_success) {
        ok = ok && !parser.failed && !parser.lexer.failed &&
             parser.lexer.token.kind == TK_END && program.count == expected_count;
        const char *registers[] = {"eax", "ebx", "ecx", "edx", "esi"};
        for (int i = 0; ok && i < program.count; i++) {
            Statement s = program.statements[i];
            ok = s.kind == ST_MOV &&
                 token_is(&source, s.operand, ordered ? registers[i] : "eax") &&
                 s.expression >= 0 && s.expression < parser.count;
            if (ok) {
                Expr e = parser.nodes[s.expression];
                ok = e.kind == EX_INT && e.value == (ordered ? i + 1 : 7) &&
                     e.left == -1 && e.right == -1;
            }
        }
    } else {
        ok = ok && parser.failed && !parser.lexer.failed;
    }
    free(program.statements);
    free(parser.nodes);
    if (!ok)
        fprintf(stderr, "Block case %s failed validation\n", argv[1]);
    return ok ? 0 : 1;
}

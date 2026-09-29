/*
 * Copyright (C) 2026 Kenneth Looney
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "kasm/program.h"

#include <stdlib.h>

static int add_statement(Parser *parser, Program *program, Statement statement) {
    if(program->count == program->capacity) {
        int new_capacity = program->capacity ? program->capacity * 2 : 4;
        Statement *new_statements = realloc(
            program->statements,
            (size_t)new_capacity * sizeof(*new_statements));
        if(!new_statements) {
            parser_error(parser, "out of memory");
            return 0;
        }
        program->statements = new_statements;
        program->capacity = new_capacity;
    }

    program->statements[program->count++] = statement;
    return 1;
}

static int take(Parser *parser, TokenKind kind, const char *message) {
    if(parser->lexer.token.kind != kind) {
        parser_error(parser, message);
        return 0;
    }
    lexer_next(&parser->lexer);
    return !parser->failed && !parser->lexer.failed;
}

static int statement(Parser *parser, Program *program) {
    Token name = parser->lexer.token;
    const Source *source = parser->lexer.cursor.source;
    if(!take(parser, TK_IDENT, "expected instruction name"))
        return 0;

    Statement s = {0};
    s.span = name.span;
    s.expression = -1;

    // mov instruction
    if(token_is(source, name, "mov")) {
        s.kind = ST_MOV;
        s.operand = parser->lexer.token;
        if(!take(parser, TK_IDENT, "expected register"))
            return 0;
        if(!take(parser, TK_COMMA, "expected comma"))
            return 0;
        s.expression = parse_expression(parser);
        if(s.expression < 0)
            return 0;
        Token semicolon = parser->lexer.token;
        if(!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
        s.span.end = semicolon.span.end;
    }
    else {
        parser_error(parser, "unknown instruction");
        return 0;
    }

    return add_statement(parser, program, s);
}

int parse_program(Parser *parser, Program *program) {
    size_t brace_depth = 0;

    program->statements = NULL;
    program->count = 0;
    program->capacity = 0;

    while(!parser->failed && !parser->lexer.failed &&
          parser->lexer.token.kind != TK_END) {
        if(parser->lexer.token.kind == TK_LBRACE) {
            brace_depth++;
            lexer_next(&parser->lexer);
        }
        else if(parser->lexer.token.kind == TK_RBRACE) {
            if(brace_depth == 0) {
                parser_error(parser, "unexpected closing brace");
                return 0;
            }
            brace_depth--;
            lexer_next(&parser->lexer);
        }
        else if(!statement(parser, program)) {
            return 0;
        }
    }

    if(brace_depth != 0) {
        parser_error(parser, "expected closing brace");
        return 0;
    }

    return !parser->failed && !parser->lexer.failed;
}

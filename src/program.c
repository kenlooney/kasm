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

static int add_statement(Parser *parser, Program *program, Statement statement)
{
    if (program->count == program->capacity)
    {
        int new_capacity = program->capacity ? program->capacity * 2 : 4;
        Statement *new_statements = realloc(
            program->statements,
            (size_t)new_capacity * sizeof(*new_statements));
        if (!new_statements)
        {
            parser_error(parser, "out of memory");
            return 0;
        }
        program->statements = new_statements;
        program->capacity = new_capacity;
    }

    program->statements[program->count++] = statement;
    return 1;
}

static int take(Parser *parser, TokenKind kind, const char *message)
{
    if (parser->lexer.token.kind != kind)
    {
        parser_error(parser, message);
        return 0;
    }
    lexer_next(&parser->lexer);
    return !parser->failed && !parser->lexer.failed;
}

static int statement(Parser *parser, Program *program)
{
    Token name = parser->lexer.token;
    const Source *source = parser->lexer.cursor.source;
    if (!take(parser, TK_IDENT, "expected instruction or label"))
        return 0;

    Statement s = {0};
    s.reg16 = REG16_INVALID;
    s.span = name.span;
    s.segment_register = SEG_INVALID;
    s.expression = -1;
    s.fill_expression = -1;
    s.far_offset = -1;
    s.mode = parser->current_mode;
    s.reg32 = REG32_INVALID;

    if (parser->lexer.token.kind == TK_COLON)
    {
        Token colon = parser->lexer.token;
        if (!take(parser, TK_COLON, "expected colon after label"))
            return 0;
        s.span.end = colon.span.end;
        s.kind = ST_LABEL;
        s.label = name;
    }
    // TODO: Add parser rejection tests when back from Doctor!
    else if (parser->lexer.token.kind == TK_IDENT &&
             token_is(source, parser->lexer.token, "equ"))
    {
        Token semicolon;

        s.kind = ST_EQU;
        s.symbol = name;

        if (!take(parser, TK_IDENT, "expected equ"))
            return 0;

        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;

        semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.span.end = semicolon.span.end;
    }

    // no-operand instructions
    else if (
        token_is(source, name, "cli") ||
        token_is(source, name, "sti") ||
        token_is(source, name, "hlt") ||
        token_is(source, name, "nop") ||
        token_is(source, name, "clc") ||
        token_is(source, name, "stc") ||
        token_is(source, name, "cmc") ||
        token_is(source, name, "cld") ||
        token_is(source, name, "std") ||
        token_is(source, name, "lahf") ||
        token_is(source, name, "sahf") ||
        token_is(source, name, "pushf") ||
        token_is(source, name, "popf") ||
        token_is(source, name, "cbw") ||
        token_is(source, name, "cwd") ||
        token_is(source, name, "iret"))
    {
        if (token_is(source, name, "cli"))
            s.kind = ST_CLI;
        else if (token_is(source, name, "sti"))
            s.kind = ST_STI;
        else if (token_is(source, name, "hlt"))
            s.kind = ST_HLT;
        else if (token_is(source, name, "clc"))
            s.kind = ST_CLC;
        else if (token_is(source, name, "stc"))
            s.kind = ST_STC;
        else if (token_is(source, name, "cmc"))
            s.kind = ST_CMC;
        else if (token_is(source, name, "cld"))
            s.kind = ST_CLD;
        else if (token_is(source, name, "std"))
            s.kind = ST_STD;
        else if (token_is(source, name, "lahf"))
            s.kind = ST_LAHF;
        else if (token_is(source, name, "sahf"))
            s.kind = ST_SAHF;
        else if (token_is(source, name, "pushf"))
            s.kind = ST_PUSHF;
        else if (token_is(source, name, "popf"))
            s.kind = ST_POPF;
        else if (token_is(source, name, "cbw"))
            s.kind = ST_CBW;
        else if (token_is(source, name, "cwd"))
            s.kind = ST_CWD;
        else if (token_is(source, name, "iret"))
            s.kind = ST_IRET;
        else
            s.kind = ST_NOP;

        Token semicolon = parser->lexer.token;

        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.span.end = semicolon.span.end;
    }
    // org instruction
    else if (token_is(source, name, "org"))
    {
        s.kind = ST_ORG;
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        Token semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
        s.span.end = semicolon.span.end;
    }
    // mode instruction
    else if (token_is(source, name, "mode"))
    {
        Token mode_token = parser->lexer.token;
        Token semicolon;
        MachineMode selected_mode;

        if (mode_token.kind != TK_NUMBER)
        {
            parser_error(parser, "expected 16, 32, or 64 after mode");
            return 0;
        }

        if (mode_token.value == 16)
            selected_mode = MODE_16;
        else if (mode_token.value == 32)
            selected_mode = MODE_32;
        else if (mode_token.value == 64)
            selected_mode = MODE_64;
        else
        {
            parser_error(parser, "mode must be 16, 32, or 64");
            return 0;
        }

        if (!take(parser, TK_NUMBER, "expected mode number"))
            return 0;

        semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.kind = ST_MODE;
        s.mode = selected_mode;
        s.span.end = semicolon.span.end;
        parser->current_mode = selected_mode;
    }

    // db instruction
    else if (token_is(source, name, "db"))
    {
        s.kind = ST_DB;
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        Token semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
        s.span.end = semicolon.span.end;
    }
    // dw instruction
    else if (token_is(source, name, "dw"))
    {
        s.kind = ST_DW;
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        Token semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
        s.span.end = semicolon.span.end;
    }
    // dd instruction
    else if (token_is(source, name, "dd"))
    {
        s.kind = ST_DD;
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        Token semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
        s.span.end = semicolon.span.end;
    }
    // Position-aware padding directive
    else if (token_is(source, name, "padto"))
    {
        s.kind = ST_PADTO;

        /* Parse the destination file offset. */
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;

        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;

        /* Parse the byte used for padding. */
        s.fill_expression = parse_expression(parser);
        if (s.fill_expression < 0)
            return 0;

        Token semicolon = parser->lexer.token;

        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.span.end = semicolon.span.end;
    }
    // Single-byte opcode with encoded register
    else if (
        token_is(source, name, "inc") ||
        token_is(source, name, "dec") ||
        token_is(source, name, "push") ||
        token_is(source, name, "pop"))
    {
        Token semicolon;
        if (token_is(source, name, "inc"))
            s.kind = ST_INC;
        else if (token_is(source, name, "dec"))
            s.kind = ST_DEC;
        else if (token_is(source, name, "push"))
            s.kind = ST_PUSH;
        else
            s.kind = ST_POP;

        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected 16-bit register after instruction"))
            return 0;
        semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
        s.span.end = semicolon.span.end;
    }
    else if (token_is(source, name, "xchg"))
    {
        Token semicolon;

        s.kind = ST_XCHG;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected ax"))
            return 0;

        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;

        s.second_operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected 16-bit register"))
            return 0;

        semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.span.end = semicolon.span.end;
    }

    // jump instructions
    else if (token_is(source, name, "jmpfar"))
    {
        Token semicolon;

        s.kind = ST_JMPFAR;
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;

        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;

        s.target = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected far-jump target"))
            return 0;

        semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.span.end = semicolon.span.end;
    }

    else if (token_is(source, name, "jmp8") ||
             token_is(source, name, "jc") ||
             token_is(source, name, "jb") ||
             token_is(source, name, "jnc") ||
             token_is(source, name, "jae"))
    {
        Token semicolon;

        if (token_is(source, name, "jmp8"))
            s.kind = ST_JMP8;
        else if (token_is(source, name, "jc") ||
                 token_is(source, name, "jb"))
            s.kind = ST_JC;
        else
            s.kind = ST_JNC;

        s.target = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected jump target"))
            return 0;

        semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.span.end = semicolon.span.end;
    }

    // mov instruction
    else if (token_is(source, name, "mov"))
    {
        Token semicolon;

        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected destination register"))
            return 0;

        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;

        if (parser->lexer.token.kind == TK_IDENT)
        {
            s.kind = ST_MOV_SEGMENT;
            s.second_operand = parser->lexer.token;
            if (!take(parser, TK_IDENT, "expected source register"))
                return 0;
        }
        else
        {
            s.kind = ST_MOV;
            s.expression = parse_expression(parser);
            if (s.expression < 0)
                return 0;
        }

        semicolon = parser->lexer.token;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;

        s.span.end = semicolon.span.end;
    }

    else
    {
        parser_error(parser, "unknown instruction");
        return 0;
    }

    return add_statement(parser, program, s);
}

int parse_program(Parser *parser, Program *program, const Target *target)
{
    if (!parser || !program || !target)
        return 0;

    parser->current_mode = target->mode;
    size_t brace_depth = 0;
    program->origin = 0;
    program->has_origin = 0;
    program->statements = NULL;
    program->count = 0;
    program->capacity = 0;

    while (!parser->failed && !parser->lexer.failed &&
           parser->lexer.token.kind != TK_END)
    {
        if (parser->lexer.token.kind == TK_LBRACE)
        {
            brace_depth++;
            lexer_next(&parser->lexer);
        }
        else if (parser->lexer.token.kind == TK_RBRACE)
        {
            if (brace_depth == 0)
            {
                parser_error(parser, "unexpected closing brace");
                return 0;
            }
            brace_depth--;
            lexer_next(&parser->lexer);
        }
        else if (!statement(parser, program))
        {
            return 0;
        }
    }

    if (brace_depth != 0)
    {
        parser_error(parser, "expected closing brace");
        return 0;
    }

    return !parser->failed && !parser->lexer.failed;
}

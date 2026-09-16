// Copyright 2026 Kenneth Looney
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "program.h"
#include <limits.h>
#include <stdlib.h>
static int take(Parser *parser, TokenKind kind, const char *message)
{
    if (parser->lexer.token.kind != kind)
    {
        parser_error(parser, message);
        return 0;
    }
    lexer_next(&parser->lexer);
    return !parser->lexer.failed;
}
static int statement(Parser *parser, Program *program)
{
    Token name = parser->lexer.token;
    const Source *source = parser->lexer.cursor.source;
    if (!take(parser, TK_IDENT, "expected instruction name"))
        return 0;
    Statement s = {0};
    s.span = name.span;
    s.expression = -1;
    if (parser->lexer.token.kind == TK_COLON)
    {
        s.kind = ST_LABEL;
        s.operand = name;
        lexer_next(&parser->lexer);
    }
    else if (token_is(source, name, "mov"))
    {
        s.kind = ST_MOV;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    else if (token_is(source, name, "ret"))
    {
        s.kind = ST_RET;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // or rim instruction
    else if (token_is(source, name, "or"))
    {
        s.kind = ST_OR;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, OR stores a register token and an expression root.
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // adc rim instruction
    else if (token_is(source, name, "adc"))
    {
        s.kind = ST_ADC;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, ADC stores a register token and an expression root.
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // int imm8 instruction
    else if (token_is(source, name, "int"))
    {
        s.kind = ST_INT_IMM8;
        // INT takes an expression directly, with no register or comma.
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // push instruction
    else if (token_is(source, name, "push"))
    {
        s.kind = ST_PUSH;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // pop instruction
    else if (token_is(source, name, "pop"))
    {
        s.kind = ST_POP;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // add rim instruction
    else if (token_is(source, name, "add"))
    {
        s.kind = ST_ADD_RIM;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, ADD stores a register token and an expression root.
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    else if (token_is(source, name, "sub"))
    {
        s.kind = ST_SUB_RIM;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, SUB stores a register token and an expression root.
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    else if (token_is(source, name, "jmp"))
    {
        Token modifier = parser->lexer.token;

        if (modifier.kind == TK_IDENT &&
            token_is(source, modifier, "near"))
            s.kind = ST_NEAR_JMP;
        else if (modifier.kind == TK_IDENT &&
                 token_is(source, modifier, "short"))
            s.kind = ST_SHORT_JMP;
        else if (modifier.kind == TK_IDENT &&
                 token_is(source, modifier, "abs"))
            s.kind = ST_ABS_JMP;
        else
        {
            parser_error(parser, "expected near, short, or abs after jmp");
            return 0;
        }

        if (!take(parser, TK_IDENT, "expected jump modifier"))
            return 0;

        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected label"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // dec instruction
    else if (token_is(source, name, "dec")) 
    {
        s.kind = ST_DEC;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected operand name"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // inc instruction
    else if (token_is(source, name, "inc"))
    {
        s.kind = ST_INC;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected operand name"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // jnz instruction
    else if (token_is(source, name, "jnz"))
    {
        s.kind = ST_JNZ;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected operand name"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // jz instruction
    else if (token_is(source, name, "jz"))
    {
        s.kind = ST_JZ;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected operand name"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    else
    {
        parser_error(parser, "unknown instruction");
        return 0;
    }
    if (program->count == program->capacity)
    {
        if (program->capacity > INT_MAX / 2)
        {
            parser_error(parser, "too many statements");
            return 0;
        }
        int capacity = program->capacity ? program->capacity * 2 : 16;
        if ((size_t)capacity > (size_t)-1 / sizeof(Statement))
        {
            parser_error(parser, "too many statements");
            return 0;
        }
        Statement *statements = realloc(program->statements,
                                        (size_t)capacity * sizeof(Statement));
        if (statements == NULL)
        {
            parser_error(parser, "out of memory");
            return 0;
        }
        program->statements = statements;
        program->capacity = capacity;
    }
    s.span.end = parser->lexer.token.span.start;
    program->statements[program->count++] = s;
    return 1;
}
static int sequence(Parser *parser, Program *program, int depth)
{
    while (!parser->failed && !parser->lexer.failed && parser->lexer.token.kind != TK_END &&
           parser->lexer.token.kind != TK_RBRACE)
    {
        if (parser->lexer.token.kind == TK_NEWLINE)
        {
            lexer_next(&parser->lexer);
            continue;
        }
        if (parser->lexer.token.kind == TK_LBRACE)
        {
            if (depth == MAX_BLOCK_DEPTH)
            {
                parser_error(parser, "blocks nested too deeply");
                return 0;
            }
            lexer_next(&parser->lexer);
            if (!sequence(parser, program, depth + 1))
                return 0;
            if (!take(parser, TK_RBRACE, "expected closing brace"))
                return 0;
        }
        else if (!statement(parser, program))
            return 0;
    }
    return !parser->failed && !parser->lexer.failed;
}
int parse_program(Parser *parser, Program *program)
{
    program->statements = NULL;
    program->count = 0;
    program->capacity = 0;
    if (!sequence(parser, program, 0))
        return 0;
    if (parser->lexer.token.kind != TK_END)
    {
        parser_error(parser, "unexpected closing brace");
        return 0;
    }
    return 1;
}

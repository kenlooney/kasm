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

static int append_data(Parser *parser, Program *program, int expression)
{
    if (program->data_count == program->data_capacity)
    {
        size_t capacity;
        if (program->data_capacity == 0)
        {
            capacity = 16;
        }
        else
        {
            if (program->data_capacity > SIZE_MAX / 2)
            {
                parser_error(parser, "too many data elements");
                return 0;
            }
            capacity = program->data_capacity * 2;
        }
        if (capacity > SIZE_MAX / sizeof(DataElement))
        {
            parser_error(parser, "too many data elements");
            return 0;
        }
        DataElement *data = realloc(program->data,
                                    capacity * sizeof(DataElement));
        if (data == NULL)
        {
            parser_error(parser, "out of memory");
            return 0;
        }
        program->data = data;
        program->data_capacity = capacity;
    }
    program->data[program->data_count++] = (DataElement){expression, 0};
    return 1;
}

static unsigned data_width_for(const Source *source, Token name)
{
    if (token_is(source, name, "db") || token_is(source, name, "byte"))
        return 1;
    if (token_is(source, name, "dw") || token_is(source, name, "word"))
        return 2;
    if (token_is(source, name, "dd") || token_is(source, name, "dword"))
        return 4;
    if (token_is(source, name, "dq") || token_is(source, name, "qword"))
        return 8;
    return 0;
}

static int parse_data(Parser *parser, Program *program,
                      Statement *s, unsigned width)
{
    s->kind = width == 1 ? ST_DB : width == 2 ? ST_DW
                               : width == 4   ? ST_DD
                                              : ST_DQ;
    s->data_width = width;
    s->data_start = program->data_count;
    for (;;)
    {
        int expression = parse_expression(parser);
        if (expression < 0)
            return 0;
        if (!append_data(parser, program, expression))
            return 0;
        s->data_count++;
        if (parser->lexer.token.kind != TK_COMMA)
            break;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
    }
    return take(parser, TK_SEMI, "expected semicolon");
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
    s.repeat_expression = -1;
    s.repeat_count = 1;
    if (parser->lexer.token.kind == TK_COLON)
    {
        s.kind = ST_LABEL;
        s.operand = name;
        lexer_next(&parser->lexer);
    }

    else if (token_is(source, name, "times") || token_is(source, name, "fill"))
    {
        s.repeat_expression = parse_expression(parser);
        if (s.repeat_expression < 0)
            return 0;
        Token directive = parser->lexer.token;
        unsigned width = directive.kind == TK_IDENT
                             ? data_width_for(source, directive)
                             : 0;
        if (width == 0)
        {
            parser_error(parser, "expected data directive after TIMES count");
            return 0;
        }
        if (!take(parser, TK_IDENT, "expected data directive"))
            return 0;
        if (!parse_data(parser, program, &s, width))
            return 0;
    }
    else if (data_width_for(source, name) != 0)
    {
        if (!parse_data(parser, program, &s, data_width_for(source, name)))
            return 0;
    }

    else if (token_is(source, name, "mov"))
    {
        s.kind = ST_MOV;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;

        if (parser->lexer.token.kind == TK_LBRACKET)
        {
            lexer_next(&parser->lexer);
            if (parser->lexer.token.kind != TK_IDENT)
            {
                parser_error(parser, "expected base register inside [ ]");
                return 0;
            }
            s.is_memory_operand = 1;
            s.base_operand = parser->lexer.token;
            lexer_next(&parser->lexer);
            if (!take(parser, TK_RBRACKET, "expected closing bracket"))
                return 0;
        }
        else
        {
            s.expression = parse_expression(parser);
            if (s.expression < 0)
                return 0;
        }

        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // halt
    else if (token_is(source, name, "halt"))
    {
        s.kind = ST_HALT;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // pause
    else if (token_is(source, name, "pause"))
    {
        s.kind = ST_PAUSE;
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
    // xor rim instruction
    else if (token_is(source, name, "xor"))
    {
        s.kind = ST_XOR;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, XOR stores a register token and an expression root.
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // and rim instruction
    else if (token_is(source, name, "and"))
    {
        s.kind = ST_AND;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, AND stores a register token and an expression root.
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
    // sbb rim instruction
    else if (token_is(source, name, "sbb"))
    {
        s.kind = ST_SBB;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, SBB stores a register token and an expression root.
        s.expression = parse_expression(parser);
        if (s.expression < 0)
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // cmp rim instruction
    else if (token_is(source, name, "cmp"))
    {
        s.kind = ST_CMP_RIM;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_COMMA, "expected comma"))
            return 0;
        // Like MOV, CMP stores a register token and an expression root.
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
    // push instructions
    else if (token_is(source, name, "push"))
    {
        s.operand = parser->lexer.token;

        if (token_is(source, s.operand, "ds"))
            s.kind = ST_PUSH_DS;
        else if (token_is(source, s.operand, "es"))
            s.kind = ST_PUSH_ES;
        else if (token_is(source, s.operand, "cs"))
            s.kind = ST_PUSH_CS;
        else if (token_is(source, s.operand, "ss"))
            s.kind = ST_PUSH_SS;
        else if (token_is(source, s.operand, "gs"))
            s.kind = ST_PUSH_GS;
        else if (token_is(source, s.operand, "fs"))
            s.kind = ST_PUSH_FS;
        else
            s.kind = ST_PUSH;

        if (!take(parser, TK_IDENT, "expected register"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // pop instructions
    else if (token_is(source, name, "pop"))
    {
        s.operand = parser->lexer.token;

        if (token_is(source, s.operand, "ds"))
            s.kind = ST_POP_DS;
        else if (token_is(source, s.operand, "es"))
            s.kind = ST_POP_ES;
        else if (token_is(source, s.operand, "cs"))
            s.kind = ST_POP_CS;
        else if (token_is(source, s.operand, "ss"))
            s.kind = ST_POP_SS;
        else if (token_is(source, s.operand, "gs"))
            s.kind = ST_POP_GS;
        else if (token_is(source, s.operand, "fs"))
            s.kind = ST_POP_FS;
        else
            s.kind = ST_POP;

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
    // jb instruction
    else if (token_is(source, name, "jb"))
    {
        s.kind = ST_JB;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected operand name"))
            return 0;
        if (!take(parser, TK_SEMI, "expected semicolon"))
            return 0;
    }
    // jl instruction
    else if (token_is(source, name, "jl"))
    {
        s.kind = ST_JL;
        s.operand = parser->lexer.token;
        if (!take(parser, TK_IDENT, "expected operand name"))
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
    // program->statements = NULL;
    // program->count = 0;
    // program->capacity = 0;
    *program = (Program){0};
    if (!sequence(parser, program, 0))
        return 0;
    if (parser->lexer.token.kind != TK_END)
    {
        parser_error(parser, "unexpected closing brace");
        return 0;
    }
    return 1;
}

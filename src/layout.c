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

#include "layout.h"
#include <string.h>
static int same_name(const Source *source, Token a, Token b)
{
    size_t n = a.span.end - a.span.start;
    return n == b.span.end - b.span.start &&
           memcmp(source->text + a.span.start, source->text + b.span.start, n) == 0;
}
size_t instruction_size(const Statement *statement)
{
    switch (statement->kind)
    {
    case ST_DB:
        return statement->data_count;
    case ST_DW:
        return statement->data_count * 2;
    case ST_DD:
        return statement->data_count * 4;
    case ST_DQ:
        return statement->data_count * 8;

    case ST_MOV:
        if (statement->is_memory_operand)
            return 2;
        return statement->operand_bits == 16 ? 3 : 5;
    case ST_ADD_RIM:
    case ST_SUB_RIM:
        if(statement->operand_bits == 16)
            return statement->reg_code == 0 ? 3 : 4;
        
        return statement->reg_code == 0 ? 5 : 6;
    case ST_RET:
        return 1;
    case ST_LABEL:
        return 0;
    case ST_NEAR_JMP:
        return 5;
    case ST_SHORT_JMP:
        return 2;
    case ST_ABS_JMP:
        return 14;
    case ST_DEC:
        return 2;
    case ST_INC:
        return 2;
    case ST_JNZ:
        return 6;
    case ST_JZ:
        return 6;
    case ST_JB:
        return 6;
    case ST_JL:
        return 6;
    case ST_PUSH:
        return 1;
    case ST_POP:
        return 1;
    case ST_OR:
        return 5;
    case ST_XOR:
        return 5;
    case ST_AND:
        return 5;
    case ST_ADC:
        return 5;
    case ST_INT_IMM8:
        return 2;
    case ST_CMP_RIM:
        return 5;
    }
    return 0;
}
static int expression_uses_location(const Parser *parser, int expression)
{
    const Expr *e = &parser->nodes[expression];
    if (e->kind == EX_CURRENT_OFFSET || e->kind == EX_SECTION_START)
        return 1;
    if (e->kind == EX_INT)
        return 0;
    return expression_uses_location(parser, e->left) ||
           expression_uses_location(parser, e->right);
}

static int evaluate_layout_expression(const Parser *parser, int expression,
                                      size_t current_offset,
                                      size_t section_start,
                                      long long *out)
{
    const Expr *e = &parser->nodes[expression];
    if (e->kind == EX_CURRENT_OFFSET)
    {
        *out = (long long)current_offset;
        return 1;
    }
    if (e->kind == EX_SECTION_START)
    {
        *out = (long long)section_start;
        return 1;
    }
    if (e->kind == EX_INT)
    {
        *out = e->value;
        return 1;
    }
    long long left, right;
    if (!evaluate_layout_expression(parser, e->left, current_offset, section_start, &left) ||
        !evaluate_layout_expression(parser, e->right, current_offset, section_start, &right))
        return 0;
    *out = e->kind == EX_ADD ? left + right :
           e->kind == EX_SUB ? left - right : left * right;
    return 1;
}

int layout(const Source *source, const Parser *parser, Program *program, const Target *target)
{
    size_t offset = 0;
    for (int i = 0; i < program->count; i++)
    {
        Statement *s = &program->statements[i];
        s->offset = offset;
        if (offset > KASM_IMAGE_LIMIT)
        {
            diagnostic(source, s->span, "image exceeds image limit");
            return 0;
        }
        size_t remaining = KASM_IMAGE_LIMIT - offset;
        size_t size;
        if (is_data_kind(s->kind))
        {
            if (s->repeat_expression >= 0 &&
                expression_uses_location(parser, s->repeat_expression))
            {
                long long count;
                if (!evaluate_layout_expression(parser, s->repeat_expression,
                                                offset, 0, &count))
                {
                    diagnostic(source, s->span, "invalid padding expression");
                    return 0;
                }
                if (count < 0 || (uint64_t)count > SIZE_MAX)
                {
                    diagnostic(source, s->span,
                               "padding count must be nonnegative and fit size_t");
                    return 0;
                }
                s->repeat_count = (size_t)count;
            }

            if (s->data_width != 1 && s->data_width != 2 &&
                s->data_width != 4 && s->data_width != 8)
            {
                diagnostic(source, s->span, "invalid data width");
                return 0;
            }
            if (s->data_count > SIZE_MAX / s->data_width)
            {
                diagnostic(source, s->span, "data size overflow");
                return 0;
            }
            size_t unit_size = s->data_count * s->data_width;
            if (unit_size != 0 && s->repeat_count > remaining / unit_size)
            {
                diagnostic(source, s->span, "repeated data exceeds image limit");
                return 0;
            }
            size = unit_size * s->repeat_count;
        }

        else
        {
            size = instruction_size(s);
            if (size > remaining)
            {
                diagnostic(source, s->span, "image exceeds image limit");
                return 0;
            }
        }
        offset += size;
        if (s->kind != ST_LABEL)
            continue;
        for (int j = 0; j < i; j++)
        {
            Statement *previous = &program->statements[j];
            if (previous->kind == ST_LABEL && same_name(source, previous->operand, s->operand))
            {
                diagnostic(source, s->operand.span, "duplicate label");
                return 0;
            }
        }
    }
    return 1;
}
int label_offset(const Source *source, const Program *program, Token name, size_t *offset)
{
    for (int i = 0; i < program->count; i++)
    {
        const Statement *s = &program->statements[i];
        if (s->kind == ST_LABEL && same_name(source, s->operand, name))
        {
            *offset = s->offset;
            return 1;
        }
    }
    diagnostic(source, name.span, "undefined label");
    return 0;
}

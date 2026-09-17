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

#include "semantic.h"

int check_program(Parser *parser, Program *program, const Target *target)
{
    if(target->mode == MODE_16)
    {
        const Source *source = parser->lexer.cursor.source;
        Span span = {0};
        diagnostic(source, span, "16-bit mode is not supported");
        return 0;
    }
    if(target->mode == MODE_32)
    {
        const Source *source = parser->lexer.cursor.source;
        Span span = {0};
        diagnostic(source, span, "32-bit mode is not supported");
        return 0;
    }
    const Source *source = parser->lexer.cursor.source;
    for (int i = 0; i < parser->count; i++)
    {
        Expr *e = &parser->nodes[i];
        if (e->kind != EX_INT)
        {
            long long left = parser->nodes[e->left].value;
            long long right = parser->nodes[e->right].value;
            if (e->kind == EX_ADD)
                e->value = left + right;
            else if (e->kind == EX_SUB)
                e->value = left - right;
            else
                e->value = left * right;
        }
        if (e->value < -2147483648LL || e->value > 2147483647LL)
        {
            diagnostic(source, e->span, "expression outside signed 32-bit range");
            return 0;
        }
    }
    for (int i = 0; i < program->count; i++)
    {
        Statement *s = &program->statements[i];
        if (s->kind == ST_DB) {
            for(size_t j = 0; j < s->data_count; j++) {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if(expression->value < 0 || expression->value > 255)
                {
                    diagnostic(source, expression->span, "declare byte element must be in range 0..255");
                    return 0;
                }
                element->value = (uint64_t)expression->value;
            
            }
            continue;
        }
        if (s->kind == ST_DW) {
            for(size_t j = 0; j < s->data_count; j++) {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if(expression->value < 0 || expression->value > 65535)
                {
                    diagnostic(source, expression->span, "declare word element must be in range 0..65535");
                    return 0;
                }
                element->value = (uint64_t)expression->value;
            }
            continue;
        }
        if (s->kind == ST_DD) {
            for(size_t j = 0; j < s->data_count; j++) {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if(expression->value < 0 || expression->value > 4294967295)
                {
                    diagnostic(source, expression->span, "declare double word element must be in range 0..4294967295");
                    return 0;
                }
                element->value = (uint64_t)expression->value;
            }
            continue;
        }
        if (s->kind == ST_DQ) {
            for(size_t j = 0; j < s->data_count; j++) {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if(expression->value < 0 || expression->value > 18446744073709551615ULL)
                {
                    diagnostic(source, expression->span, "declare quad word element must be in range 0..18446744073709551615");
                    return 0;
                }
                element->value = (uint64_t)expression->value;
            }
            continue;
        }
        if (s->kind == ST_PUSH || s->kind == ST_POP)
        {
            if (!token_is(source, s->operand, "rax"))
            {
                diagnostic(source, s->operand.span,
                           "push/pop require register rax");
                return 0;
            }
        }
        else if (s->kind == ST_MOV || s->kind == ST_DEC ||
                 s->kind == ST_INC || s->kind == ST_ADD_RIM ||
                 s->kind == ST_SUB_RIM || s->kind == ST_OR ||
                 s->kind == ST_ADC || s->kind == ST_CMP_RIM || s->kind == ST_AND || s->kind == ST_XOR)
        {
            if (token_is(source, s->operand, "eax"))
                s->reg_code = 0; // Assuming 0 corresponds to eax
            else if (token_is(source, s->operand, "ecx"))
                s->reg_code = 1;
            else if (token_is(source, s->operand, "edx"))
                s->reg_code = 2;
            else
            {
                diagnostic(source, s->operand.span, "expected eax, ecx, or edx");
                return 0;
            }
        }
        if (s->kind == ST_MOV || s->kind == ST_ADD_RIM || s->kind == ST_SUB_RIM || s->kind == ST_OR || s->kind == ST_ADC || s->kind == ST_INT_IMM8 || s->kind == ST_CMP_RIM || s->kind == ST_AND || s->kind == ST_XOR)
        {
            s->value = parser->nodes[s->expression].value;
            if (s->kind == ST_INT_IMM8 && (s->value < 0 || s->value > 255))
            {
                diagnostic(source, parser->nodes[s->expression].span,
                           "interrupt vector must be in range 0..255");
                return 0;
            }
            // ADD accepts signed results; MOV keeps its existing restriction.
            if (s->kind == ST_MOV && s->value < 0)
            {
                diagnostic(source, parser->nodes[s->expression].span,
                           "mov immediate must be nonnegative in this language");
                return 0;
            }
        }
    }
    return 1;
}

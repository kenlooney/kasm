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

int expression_uses_location(const Parser *parser, int expression)
{
    const Expr *e = &parser->nodes[expression];
    if (e->kind == EX_CURRENT_OFFSET || e->kind == EX_SECTION_START)
        return 1;
    if (e->kind == EX_INT)
        return 0;
    return expression_uses_location(parser, e->left) ||
           expression_uses_location(parser, e->right);
}

int check_program(Parser *parser, Program *program, const Target *target)
{

    if (target->mode == MODE_32)
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

        if (e->kind == EX_CURRENT_OFFSET || e->kind == EX_SECTION_START)
            continue;

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

        if (is_data_kind(s->kind) && s->repeat_expression >= 0)
        {
            if (expression_uses_location(parser, s->repeat_expression))
            {
                /* resolved later in layout() */
            }
            else
            {
                Expr *count = &parser->nodes[s->repeat_expression];
                if (count->value < 0 || (uint64_t)count->value > SIZE_MAX)
                {
                    diagnostic(source, count->span,
                               "TIMES/FILL count must be nonnegative and fit size_t");
                    return 0;
                }
                s->repeat_count = (size_t)count->value;
            }
        }

        if (s->kind == ST_DB)
        {
            for (size_t j = 0; j < s->data_count; j++)
            {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if (expression->value < 0 || expression->value > 255)
                {
                    diagnostic(source, expression->span, "declare byte element must be in range 0..255");
                    return 0;
                }
                element->value = (uint64_t)expression->value;
            }
            continue;
        }
        if (s->kind == ST_DW)
        {
            for (size_t j = 0; j < s->data_count; j++)
            {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if (expression->value < 0 || expression->value > 65535)
                {
                    diagnostic(source, expression->span, "declare word element must be in range 0..65535");
                    return 0;
                }
                element->value = (uint64_t)expression->value;
            }
            continue;
        }
        if (s->kind == ST_DD)
        {
            for (size_t j = 0; j < s->data_count; j++)
            {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if (expression->value < 0 || expression->value > 4294967295)
                {
                    diagnostic(source, expression->span, "declare double word element must be in range 0..4294967295");
                    return 0;
                }
                element->value = (uint64_t)expression->value;
            }
            continue;
        }
        if (s->kind == ST_DQ)
        {
            for (size_t j = 0; j < s->data_count; j++)
            {
                DataElement *element = &program->data[s->data_start + j];
                Expr *expression = &parser->nodes[element->expression];
                if (expression->value < 0 || expression->value > 18446744073709551615ULL)
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
        // push/pop cs
        if (s->kind == ST_PUSH_CS || s->kind == ST_POP_CS)
        {
            if (!token_is(source, s->operand, "cs"))
            {
                diagnostic(source, s->operand.span,
                           "push/pop require segment register cs");
                return 0;
            }
        }
        // push/pop gs
        if (s->kind == ST_PUSH_GS || s->kind == ST_POP_GS)
        {
            if (!token_is(source, s->operand, "gs"))
            {
                diagnostic(source, s->operand.span,
                           "push/pop require segment register gs");
                return 0;
            }
        }
        // push/pop fs
        if (s->kind == ST_PUSH_FS || s->kind == ST_POP_FS)
        {
            if (!token_is(source, s->operand, "fs"))
            {
                diagnostic(source, s->operand.span,
                           "push/pop require segment register fs");
                return 0;
            }
        }
        // push/pop ss
        if (s->kind == ST_PUSH_SS || s->kind == ST_POP_SS)
        {
            if (!token_is(source, s->operand, "ss"))
            {
                diagnostic(source, s->operand.span,
                           "push/pop require segment register ss");
                return 0;
            }
        }
        // push/pop es
        if (s->kind == ST_PUSH_ES || s->kind == ST_POP_ES)
        {
            if (!token_is(source, s->operand, "es"))
            {
                diagnostic(source, s->operand.span,
                           "push/pop require segment register es");
                return 0;
            }
        }
        // push/pop ds
        if (s->kind == ST_PUSH_DS || s->kind == ST_POP_DS)
        {
            if (!token_is(source, s->operand, "ds"))
            {
                diagnostic(source, s->operand.span,
                           "push/pop require segment register ds");
                return 0;
            }
        }

        if (s->kind == ST_MOV)
        {
            if (s->is_memory_operand)
            {
                if (target->mode != MODE_16)
                {
                    diagnostic(source, s->operand.span,
                               "memory operands are only supported in MODE_16");
                    return 0;
                }
                if (!token_is(source, s->base_operand, "bx"))
                {
                    diagnostic(source, s->base_operand.span,
                               "only [bx] is supported in this chapter");
                    return 0;
                }
            }
            
            if (token_is(source, s->operand, "eax"))
            {
                s->reg_code = 0;      // Assuming 0 corresponds to eax
                s->operand_bits = 32; // eax is a 32-bit register
            }
            else if (token_is(source, s->operand, "ax"))
            {
                s->reg_code = 0;      // Assuming 0 corresponds to ax
                s->operand_bits = 16; // ax is a 16-bit register
            }

            else if (token_is(source, s->operand, "ecx"))
            {
                s->reg_code = 1;      // Assuming 1 corresponds to ecx
                s->operand_bits = 32; // ecx is a 32-bit register
            }
            else if (token_is(source, s->operand, "cx"))
            {
                s->reg_code = 1;      // Assuming 1 corresponds to cx
                s->operand_bits = 16; // cx is a 16-bit register
            }
            else if (token_is(source, s->operand, "edx"))
            {
                s->reg_code = 2;      // Assuming 2 corresponds to edx
                s->operand_bits = 32; // edx is a 32-bit register
            }
            else if (token_is(source, s->operand, "dx"))
            {
                s->reg_code = 2;      // Assuming 2 corresponds to dx
                s->operand_bits = 16; // dx is a 16-bit register
            }
            else
            {
                diagnostic(source, s->operand.span,
                           "expected eax, ax, ecx, cx, edx, or dx");
                return 0;
            }
            if (target->mode == MODE_16 && s->operand_bits != 16)
            {
                diagnostic(source, s->operand.span,
                           "16-bit mode requires 16-bit registers: ax, cx, or dx");
                return 0;
            }
            if (target->mode != MODE_16 && s->operand_bits != 32)
            {
                diagnostic(source, s->operand.span,
                           "This target requires 32-bit registers: eax, ecx, or edx");
                return 0;
            }
        }
        if (s->kind == ST_MOV && !s->is_memory_operand &&
            s->operand_bits == 16 && s->value > 65535)
        {
            diagnostic(source, parser->nodes[s->expression].span,
                       "mov ax/cx/dx immediate must fit in 16 bits");
            return 0;
        }

        else if (s->kind == ST_ADD_RIM || s->kind == ST_SUB_RIM ||
             s->kind == ST_SBB)
        {
            if (token_is(source, s->operand, "eax"))
            {
                s->reg_code = 0;      // Assuming 0 corresponds to eax
                s->operand_bits = 32; // eax is a 32-bit register
            }
            else if (token_is(source, s->operand, "ax"))
            {
                s->reg_code = 0;      // Assuming 0 corresponds to ax
                s->operand_bits = 16; // ax is a 16-bit register
            }
            else if (token_is(source, s->operand, "ecx"))
            {
                s->reg_code = 1;      // Assuming 1 corresponds to ecx
                s->operand_bits = 32; // ecx is a 32-bit register
            }
            else if (token_is(source, s->operand, "cx"))
            {
                s->reg_code = 1;      // Assuming 1 corresponds to cx
                s->operand_bits = 16; // cx is a 16-bit register
            }
            else if (token_is(source, s->operand, "edx"))
            {
                s->reg_code = 2;      // Assuming 2 corresponds to edx
                s->operand_bits = 32; // edx is a 32-bit register
            }
            else if (token_is(source, s->operand, "dx"))
            {
                s->reg_code = 2;      // Assuming 2 corresponds to dx
                s->operand_bits = 16; // dx is a 16-bit register
            }
            else
            {
                diagnostic(source, s->operand.span,
                           "expected eax, ecx, edx, ax, cx, or dx");
                return 0;
            }

            if (target->mode == MODE_16 && s->operand_bits != 16)
            {
                diagnostic(source, s->operand.span,
                           "MODE_16 requires ax, cx, or dx");
                return 0;
            }

            if (target->mode != MODE_16 && s->operand_bits != 32)
            {
                diagnostic(source, s->operand.span,
                           "this target requires eax, ecx, or edx");
                return 0;
            }
        }
        else if (s->kind == ST_DEC ||
                 s->kind == ST_INC ||
                 s->kind == ST_OR ||
                 s->kind == ST_ADC || s->kind == ST_SBB || s->kind == ST_CMP_RIM || s->kind == ST_AND || s->kind == ST_XOR)
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
        if ((s->kind == ST_MOV && !s->is_memory_operand) ||
            s->kind == ST_ADD_RIM || s->kind == ST_SUB_RIM ||
            s->kind == ST_OR || s->kind == ST_ADC || s->kind == ST_SBB ||
            s->kind == ST_INT_IMM8 || s->kind == ST_CMP_RIM ||
            s->kind == ST_AND || s->kind == ST_XOR)
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

#include "kasm/program.h"
#include <string.h>
#include <stdint.h>

static int tokens_equal(
    const Source *source,
    Token left,
    Token right)
{
    size_t left_length = left.span.end - left.span.start;
    size_t right_length = right.span.end - right.span.start;

    return left_length == right_length &&
           memcmp(
               source->text + left.span.start,
               source->text + right.span.start,
               left_length) == 0;
}

static int register16_from_token(
    const Source *source,
    Token token,
    Register16 *reg)

{
    if(!reg)
    return 0;

    if (token_is(source, token, "ax"))
        *reg = REG16_AX;
    else if (token_is(source, token, "cx"))
        *reg = REG16_CX;
    else if (token_is(source, token, "dx"))
        *reg = REG16_DX;
    else if (token_is(source, token, "bx"))
        *reg = REG16_BX;
    else if (token_is(source, token, "sp"))
        *reg = REG16_SP;
    else if (token_is(source, token, "bp"))
        *reg = REG16_BP;
    else if (token_is(source, token, "si"))
        *reg = REG16_SI;
    else if (token_is(source, token, "di"))
        *reg = REG16_DI;
    else
        return 0;

    return 1;
}

int evaluate_program(Parser *parser, Program *program)
{
    if (!parser || !program)
        return 0;

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
            else if (e->kind == EX_MUL)
                e->value = left * right;
            else if (e->kind == EX_DIV || e->kind == EX_MOD)
            {
                if (right == 0)
                {
                    diagnostic(source, e->span, "division by zero");
                    return 0;
                }
                e->value = e->kind == EX_DIV ? left / right : left % right;
            }
        }
        if (e->value < -2147483648LL || e->value > 2147483647LL)
        {
            diagnostic(source, e->span, "expression outside signed 32-bit range");
            return 0;
        }
    }

    for (int i = 0; i < program->count; i++)
    {
        Statement *statement = &program->statements[i];

        if (statement->expression >= 0)
            statement->value = parser->nodes[statement->expression].value;

        if (statement->fill_expression >= 0)
            statement->fill_value =
                parser->nodes[statement->fill_expression].value;
    }

    return 1;
}

int check_program(Parser *parser, Program *program, const Target *target)
{
    if (!target)
        return 0;

    const Source *source = parser->lexer.cursor.source;

    for (int i = 0; i < program->count; i++)
    {
        Statement *s = &program->statements[i];

        if (target->arch != ARCH_X86 || target->mode != MODE_16)
        {
            diagnostic(source, s->span, "only 16-bit x86 mode is supported");
            return 0;
        }

        if (s->kind == ST_MOV)
        {
            if (!token_is(source, s->operand, "ax"))
            {
                diagnostic(source, s->operand.span, "only register ax is supported");
                return 0;
            }
            // TODO: Select the immediate range from the target mode when 32-bit
            // and 64-bit instruction encoding is supported.
            if (s->value < 0 || s->value > 65535)
            {
                diagnostic(source,
                           parser->nodes[s->expression].span,
                           "16-bit immediate must be in range 0..65535");
                return 0;
            }
        }
        // Single-byte opcode with encoded register
        else if(s->kind == ST_INC)
        {
            if(!register16_from_token(source, s->operand, &s->reg16))
            {
                diagnostic(source, s->operand.span, "expected ax, cx, dx, bx, sp, bp, si, or di");
                return 0;
            }
        }
        
        else if (s->kind == ST_JMP8)
        {
            const Statement *destination = NULL;

            /*
             * Search every statement for a label whose name matches
             * the jump's target.
             */
            for (int j = 0; j < program->count; j++)
            {
                const Statement *candidate = &program->statements[j];

                if (candidate->kind == ST_LABEL &&
                    tokens_equal(source, s->target, candidate->label))
                {
                    destination = candidate;
                    break;
                }
            }

            if (!destination)
            {
                diagnostic(source, s->target.span, "undefined jump target");
                return 0;
            }

            /*
             * JMP8 is two bytes, and its displacement is measured from
             * the address immediately after the instruction.
             */
            long long displacement =
                (long long)destination->offset -
                ((long long)s->offset + 2);

            if (displacement < -128 || displacement > 127)
            {
                diagnostic(
                    source,
                    s->target.span,
                    "short jump target is out of range");
                return 0;
            }

            s->value = displacement;
        }
        // ORG
        else if (s->kind == ST_ORG)
        {
            if (s->offset != 0)
            {
                diagnostic(
                    source,
                    s->span,
                    "origin must appear before emitted content");
                return 0;
            }
            if (s->value < 0)
            {
                diagnostic(source, s->span, "origin must be nonnegative");
                return 0;
            }
            if (program->has_origin)
            {
                diagnostic(source, s->span, "origin already specified");
                return 0;
            }

            program->origin = (uint64_t)s->value;
            program->has_origin = 1;
        }

        // data definition instructions (db, dw, dd)
        else if (s->kind == ST_DB ||
                 s->kind == ST_DW ||
                 s->kind == ST_DD)
        {
            long long maximum;
            // TODO: Later add 64bit `dq`
            if (s->kind == ST_DB)
                maximum = (long long)UINT8_MAX;
            else if (s->kind == ST_DW)
                maximum = (long long)UINT16_MAX;
            else
                maximum = (long long)UINT32_MAX;
            if (s->value < 0 || s->value > maximum)
            {
                diagnostic(source,
                           parser->nodes[s->expression].span,
                           "data value does not fit its declared width");
                return 0;
            }
        }
        else if (s->kind == ST_PADTO)
        {
            if (s->value < 0)
            {
                diagnostic(source, s->span,
                           "padto target must be nonnegative");
                return 0;
            }

            if ((size_t)s->value < s->offset)
            {
                diagnostic(source, s->span,
                           "padto target is before the current offset");
                return 0;
            }

            if (s->fill_value < 0 || s->fill_value > UINT8_MAX)
            {
                diagnostic(source,
                           parser->nodes[s->fill_expression].span,
                           "padto fill value must be in range 0..255");
                return 0;
            }
        }
    }
    return 1;
}

#include "kasm/program.h"
#include <string.h>

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

int check_program(Parser *parser, Program *program, const Target *target)
{

    if (!target)
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
            s->value = parser->nodes[s->expression].value;
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
    }
    return 1;
}

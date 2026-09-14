#include "semantic.h"

int check_program(Parser *parser, Program *program)
{
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
        if (s->kind == ST_MOV)
        {

            if (!token_is(source, s->operand, "eax"))
            {
                diagnostic(source, s->operand.span, "only register eax is supported");
                return 0;
            }
        }
        if (s->kind == ST_MOV)
        {
            s->value = parser->nodes[s->expression].value;
            if (s->value < 0)
            {
                diagnostic(source, parser->nodes[s->expression].span,
                           "mov immediate must be nonnegative in this language");
                return 0;
            }
        }
    }
    return 1;
}
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
static int expression_uses_symbol(const Parser *parser, int root)
{
    const Expr *expression;

    if (!parser || root < 0 || root >= parser->count)
        return 0;

    expression = &parser->nodes[root];

    if (expression->kind == EX_SYMBOL)
        return 1;
    if (expression->kind == EX_INT)
        return 0;

    return expression_uses_symbol(parser, expression->left) ||
           expression_uses_symbol(parser, expression->right);
}

static int register16_from_token(
    const Source *source,
    Token token,
    Register16 *reg)

{
    if (!reg)
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
static int register32_from_token(
    const Source *source,
    Token token,
    Register32 *reg)
{
    if (!reg)
        return 0;

    if (token_is(source, token, "eax"))
        *reg = REG32_EAX;
    else if (token_is(source, token, "ecx"))
        *reg = REG32_ECX;
    else if (token_is(source, token, "edx"))
        *reg = REG32_EDX;
    else if (token_is(source, token, "ebx"))
        *reg = REG32_EBX;
    else if (token_is(source, token, "esp"))
        *reg = REG32_ESP;
    else if (token_is(source, token, "ebp"))
        *reg = REG32_EBP;
    else if (token_is(source, token, "esi"))
        *reg = REG32_ESI;
    else if (token_is(source, token, "edi"))
        *reg = REG32_EDI;
    else
        return 0;

    return 1;
}

static int segment_register_from_token(
    const Source *source,
    Token token,
    SegmentRegister *segment_register)
{
    if (!segment_register)
        return 0;

    if (token_is(source, token, "es"))
        *segment_register = SEG_ES;
    else if (token_is(source, token, "ss"))
        *segment_register = SEG_SS;
    else if (token_is(source, token, "ds"))
        *segment_register = SEG_DS;
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
        if (expression_uses_symbol(parser, i))
            continue;

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
        {
            if (expression_uses_symbol(parser, statement->expression))
            {
                if (statement->kind == ST_ORG ||
                    statement->kind == ST_PADTO)
                {
                    diagnostic(source,
                               parser->nodes[statement->expression].span,
                               "layout directives require constant expressions");
                    return 0;
                }
            }
            else
            {
                statement->value =
                    parser->nodes[statement->expression].value;
            }
        }

        if (statement->fill_expression >= 0)
        {
            if (expression_uses_symbol(parser, statement->fill_expression))
            {
                diagnostic(source,
                           parser->nodes[statement->fill_expression].span,
                           "padto fill requires a constant expression");
                return 0;
            }

            statement->fill_value =
                parser->nodes[statement->fill_expression].value;
        }
    }

    return 1;
}

static int establish_origin(const Source *source, Program *program)
{
    program->origin = 0;
    program->has_origin = 0;

    for (int i = 0; i < program->count; i++)
    {
        Statement *statement = &program->statements[i];

        if (statement->kind != ST_ORG)
            continue;

        if (statement->offset != 0)
        {
            diagnostic(source,
                       statement->span,
                       "origin must appear before emitted content");
            return 0;
        }

        if (statement->value < 0)
        {
            diagnostic(source,
                       statement->span,
                       "origin must be nonnegative");
            return 0;
        }

        if (program->has_origin)
        {
            diagnostic(source,
                       statement->span,
                       "origin already specified");
            return 0;
        }

        program->origin = (uint64_t)statement->value;
        program->has_origin = 1;
    }

    return 1;
}

static int resolve_symbol_expression(
    const Source *source,
    const Parser *parser,
    const Program *program,
    int root,
    int allow_equ,
    long long *result)
{
    const Expr *expression = &parser->nodes[root];
    long long left;
    long long right;

    if (expression->kind == EX_INT)
    {
        *result = expression->value;
        return 1;
    }

    if (expression->kind == EX_SYMBOL)
    {
        Token name = {TK_IDENT, expression->span, 0};

        for (int i = 0; i < program->count; i++)
        {
            const Statement *candidate = &program->statements[i];
            uint64_t address;

            if (candidate->kind != ST_LABEL ||
                !tokens_equal(source, name, candidate->label))
                continue;

            if (candidate->offset > UINT64_MAX - program->origin)
                return 0;

            address = program->origin + candidate->offset;
            if (address > INT32_MAX)
            {
                diagnostic(source,
                           expression->span,
                           "label address is outside expression range");
                return 0;
            }

            *result = (long long)address;
            return 1;
        }
        if (allow_equ)
        {
            for (int i = 0; i < program->count; i++)
            {
                const Statement *candidate = &program->statements[i];

                if (candidate->kind == ST_EQU &&
                    tokens_equal(source, name, candidate->symbol))
                {
                    *result = candidate->value;
                    return 1;
                }
            }
        }

        diagnostic(source, expression->span, "undefined symbol");
        return 0;
    }

    if (!resolve_symbol_expression(
            source, parser, program, expression->left, allow_equ, &left) ||
        !resolve_symbol_expression(
            source, parser, program, expression->right, allow_equ, &right))
        return 0;

    if (expression->kind == EX_ADD)
        *result = left + right;
    else if (expression->kind == EX_SUB)
        *result = left - right;
    else if (expression->kind == EX_MUL)
        *result = left * right;
    else if (expression->kind == EX_DIV || expression->kind == EX_MOD)
    {
        if (right == 0)
        {
            diagnostic(source, expression->span, "division by zero");
            return 0;
        }

        *result = expression->kind == EX_DIV
                      ? left / right
                      : left % right;
    }
    else
    {
        return 0;
    }

    if (*result < INT32_MIN || *result > INT32_MAX)
    {
        diagnostic(source,
                   expression->span,
                   "expression outside signed 32-bit range");
        return 0;
    }

    return 1;
}

static int statement_defines_symbol(const Statement *statement)
{
    return statement->kind == ST_LABEL || statement->kind == ST_EQU;
}

static Token statement_symbol(const Statement *statement)
{
    return statement->kind == ST_LABEL
               ? statement->label
               : statement->symbol;
}

static int check_duplicate_symbols(
    const Source *source,
    const Program *program)
{
    for (int i = 0; i < program->count; i++)
    {
        const Statement *current = &program->statements[i];

        if (!statement_defines_symbol(current))
            continue;

        for (int j = 0; j < i; j++)
        {
            const Statement *previous = &program->statements[j];

            if (statement_defines_symbol(previous) &&
                tokens_equal(
                    source,
                    statement_symbol(current),
                    statement_symbol(previous)))
            {
                diagnostic(source,
                           statement_symbol(current).span,
                           "duplicate symbol");
                return 0;
            }
        }
    }

    return 1;
}

int check_program(Parser *parser, Program *program, const Target *target)
{
    if (!parser || !program || !target || target->arch != ARCH_X86)
        return 0;

    const Source *source = parser->lexer.cursor.source;
    if (!establish_origin(source, program))
        return 0;
    if (!check_duplicate_symbols(source, program))
        return 0;

    /*
     * First resolve EQU definitions from literals and labels. EQU-to-EQU
     * dependencies are deliberately not enabled by this first implementation.
     */
    for (int i = 0; i < program->count; i++)
    {
        Statement *statement = &program->statements[i];

        if (statement->kind != ST_EQU)
            continue;

        if (!resolve_symbol_expression(
                source,
                parser,
                program,
                statement->expression,
                0,
                &statement->value))
        {
            diagnostic(
                source,
                statement->symbol.span,
                "equ expression must use literals and labels");
            return 0;
        }
    }

    /*
     * Then resolve uses in ordinary statements. At this point every EQU
     * statement already has its final value, so those names are visible.
     */
    for (int i = 0; i < program->count; i++)
    {
        Statement *statement = &program->statements[i];

        if (statement->kind == ST_EQU ||
            statement->expression < 0 ||
            !expression_uses_symbol(parser, statement->expression))
            continue;

        if (!resolve_symbol_expression(
                source,
                parser,
                program,
                statement->expression,
                1,
                &statement->value))
            return 0;
    }

    for (int i = 0; i < program->count; i++)
    {
        Statement *s = &program->statements[i];

        if (s->mode != MODE_16 &&
            s->kind != ST_MODE &&
            s->kind != ST_EQU &&
            s->kind != ST_LABEL &&
            s->kind != ST_ORG &&
            s->kind != ST_DB &&
            s->kind != ST_DW &&
            s->kind != ST_DD &&
            s->kind != ST_PADTO &&
            s->kind != ST_MOV &&
            s->kind != ST_MOV_SEGMENT)
        {
            diagnostic(source, s->span,
                       "instruction is not implemented in this mode");
            return 0;
        }

        if (s->kind == ST_MOV)
        {
            if (s->mode == MODE_16)
            {
                if (!register16_from_token(source, s->operand, &s->reg16))
                {
                    diagnostic(source, s->operand.span,
                               "expected a 16-bit register in mode 16");
                    return 0;
                }

                if (s->value < 0 || s->value > UINT16_MAX)
                {
                    diagnostic(source, parser->nodes[s->expression].span,
                               "16-bit immediate must be in range 0..65535");
                    return 0;
                }
            }
            else if (s->mode == MODE_32)
            {
                if (!register32_from_token(source, s->operand, &s->reg32))
                {
                    diagnostic(source, s->operand.span,
                               "expected a 32-bit register in mode 32");
                    return 0;
                }

                if (s->value < 0 || (uint64_t)s->value > UINT32_MAX)
                {
                    diagnostic(source, parser->nodes[s->expression].span,
                               "32-bit immediate must fit in 32 bits");
                    return 0;
                }
            }
            else
            {
                diagnostic(source, s->span,
                           "64-bit MOV is not implemented yet");
                return 0;
            }
        }

        else if (s->kind == ST_MOV_SEGMENT)
        {
            if (s->mode != MODE_16)
            {
                diagnostic(source, s->span,
                           "segment-register MOV currently requires mode 16");
                return 0;
            }

            if (!segment_register_from_token(
                    source,
                    s->operand,
                    &s->segment_register))
            {
                diagnostic(source,
                           s->operand.span,
                           "expected es, ss, or ds");
                return 0;
            }

            if (!register16_from_token(source, s->second_operand, &s->reg16) ||
                s->reg16 != REG16_AX)
            {
                diagnostic(source,
                           s->second_operand.span,
                           "this segment-register form requires ax as the source");
                return 0;
            }
        }

        // Single-byte opcode with encoded register
        else if (s->kind == ST_INC ||
                 s->kind == ST_DEC ||
                 s->kind == ST_PUSH ||
                 s->kind == ST_POP)
        {
            if (!register16_from_token(source, s->operand, &s->reg16))
            {
                diagnostic(source,
                           s->operand.span,
                           "expected ax, cx, dx, bx, sp, bp, si, or di");
                return 0;
            }
        }
        else if (s->kind == ST_XCHG)
        {
            if (!token_is(source, s->operand, "ax"))
            {
                diagnostic(source,
                           s->operand.span,
                           "this form of xchg requires ax first");
                return 0;
            }

            if (!register16_from_token(
                    source,
                    s->second_operand,
                    &s->reg16))
            {
                diagnostic(source,
                           s->second_operand.span,
                           "expected a 16-bit register after the comma");
                return 0;
            }
        }

        else if (s->kind == ST_JMP8 ||
                 s->kind == ST_JC ||
                 s->kind == ST_JNC)

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
        else if (s->kind == ST_JMPFAR)
        {
            const Statement *destination = NULL;
            uint64_t address;

            if (s->value < 0 || s->value > UINT16_MAX)
            {
                diagnostic(source,
                           parser->nodes[s->expression].span,
                           "far-jump segment must fit in 16 bits");
                return 0;
            }

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
                diagnostic(source, s->target.span, "undefined far-jump target");
                return 0;
            }

            if (program->origin > UINT16_MAX ||
                destination->offset > UINT16_MAX - program->origin)
            {
                diagnostic(source,
                           s->target.span,
                           "far-jump offset must fit in 16 bits");
                return 0;
            }

            address = program->origin + destination->offset;
            s->far_offset = (long long)address;
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

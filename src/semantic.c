#include "kasm/program.h"

int check_program(Parser *parser, Program *program, const Target *target) {
    
    if(!target) return 0;

    const Source *source = parser->lexer.cursor.source;
    for (int i = 0; i < parser->count; i++) {
        Expr *e = &parser->nodes[i];
        if (e->kind != EX_INT) {
            long long left = parser->nodes[e->left].value;
            long long right = parser->nodes[e->right].value;
            if (e->kind == EX_ADD)
                e->value = left + right;
            else if (e->kind == EX_SUB)
                e->value = left - right;
            else if (e->kind == EX_MUL)
                e->value = left * right;
            else if (e->kind == EX_DIV || e->kind == EX_MOD) {
                if (right == 0) {
                    diagnostic(source, e->span, "division by zero");
                    return 0;
                }
                e->value = e->kind == EX_DIV ? left / right : left % right;
            }
        }
        if (e->value < -2147483648LL || e->value > 2147483647LL) {
            diagnostic(source, e->span, "expression outside signed 32-bit range");
            return 0;
        }
    }
    for (int i = 0; i < program->count; i++) {
        Statement *s = &program->statements[i];
        
        if(target->arch !=  ARCH_X86 || target->mode != MODE_16) {
            diagnostic(source, s->operand.span, "only 16-bit x86 mode is supported");
            return 0;
        }
        
        if (s->kind == ST_MOV) {
            if (!token_is(source, s->operand, "ax")) {
                diagnostic(source, s->operand.span, "only register ax is supported");
                return 0;
        }
        s->value = parser->nodes[s->expression].value;
        // TODO: Select the immediate range from the target mode when 32-bit
        // and 64-bit instruction encoding is supported.
        if (s->value < 0 || s->value > 65535) {
            diagnostic(source, 
                parser->nodes[s->expression].span,
                "16-bit immediate must be in range 0..65535"
            );
                return 0;
            }
        }
    }
    return 1;
}

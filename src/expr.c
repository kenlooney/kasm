#include "kasm/expr.h"

#include <stdlib.h>

/*
 * Initialize the parser with the given source.
 */
void parser_start(Parser *parser, const Source *source) {
    parser->nodes = NULL;
    parser->count = 0;
    parser->depth = 0;
    parser->failed = 0;
    lexer_start(&parser->lexer, source);
}

/*
 * Report a parser error with the given message.
 */
void parser_error(Parser *parser, const char *message) {
    if(!parser->failed && !parser->lexer.failed)
        diagnostic(parser->lexer.cursor.source,parser->lexer.token.span, message);
    parser->failed = 1;
}

/*
 * Add a new node to the parser's list of expressions.
 * Returns the index of the new node, or -1 on failure.
 */
static int node(Parser *parser, Expr expression) {
    if(parser->count == parser->depth) {
        int new_depth = parser->depth ? parser->depth * 2 : 4;
        Expr *new_nodes = realloc(
            parser->nodes, (size_t)new_depth * sizeof(*new_nodes));
        if(!new_nodes) {
            parser_error(parser, "Out of memory");
            return -1;
        }
        parser->nodes = new_nodes;
        parser->depth = new_depth;
    }
    parser->nodes[parser->count] = expression;
    return parser->count++;
}

static int primary(Parser *parser){
    Token token = parser->lexer.token;

    if(token.kind != TK_NUMBER || parser->lexer.failed) {
        parser_error(parser, "Expected a number");
        return -1;
    }
    Expr expression = { EX_INT, token.span, token.value, -1, -1 };
    lexer_next(&parser->lexer);
    return node(parser, expression);
}

static int binary(Parser *parser, ExprKind kind, int left, int right)
{
    if(left < 0 || right < 0) {
        return -1;
    }
    Span span = { parser->nodes[left].span.start, parser->nodes[right].span.end };
    Expr expression = { kind, span, 0, left, right };
    return node(parser, expression);
}

int parse_expression(Parser *parser) {
    int left = primary(parser);

    while(left >= 0 && (parser->lexer.token.kind == TK_PLUS || parser->lexer.token.kind == TK_MINUS)) {
        TokenKind op = parser->lexer.token.kind;
        lexer_next(&parser->lexer);
        int right = primary(parser);
        left = binary(parser, op == TK_PLUS ? EX_ADD : EX_SUB, left, right);
    }
    return left;
}

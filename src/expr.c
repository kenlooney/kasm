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

#include "expr.h"
#include <stdlib.h>

void parser_start(Parser *parser, const Source *source) {
    parser->nodes = NULL;
    parser->count = 0;
    parser->depth = 0;
    parser->failed = 0;
    lexer_start(&parser->lexer, source);
}

void parser_error(Parser *parser, const char *message) {
   if (!parser->failed && !parser->lexer.failed) {
        diagnostic(parser->lexer.cursor.source, parser->lexer.token.span, message);
        parser->failed = 1;
   }
}

static int node(Parser *parser, Expr expression) {
    Expr *nodes = realloc(parser->nodes, sizeof(Expr) * (parser->count + 1));
    if (nodes == NULL) {
        parser_error(parser, "out of memory");
        return -1;
    }
    parser->nodes = nodes;
    parser->nodes[parser->count] = expression;
    return parser->count++;
}
static int primary(Parser *parser) {
    Token token = parser->lexer.token;

     if (token.kind == TK_LPAREN) {
        if (parser->depth == 32) {
            parser_error(parser, "parentheses nested too deeply");
            return -1;
        }
        parser->depth++;
        lexer_next(&parser->lexer);
        int root = parse_expression(parser);
        parser->depth--;
        if (root < 0)
            return -1;
        if (parser->lexer.token.kind != TK_RPAREN) {
            parser_error(parser, "expected closing parenthesis");
            return -1;
        }
        parser->nodes[root].span.start = token.span.start;
        parser->nodes[root].span.end = parser->lexer.token.span.end;
        lexer_next(&parser->lexer);
        return root;
    }
    
    if (token.kind != TK_NUMBER || parser->lexer.failed) {
        parser_error(parser, "expected integer literal");
        return -1;
    }
    
    Expr expression;
    expression.kind = EX_INT;
    expression.span = parser->lexer.token.span;
    expression.value = parser->lexer.token.value;
    expression.left = -1;
    expression.right = -1;
    int result = node(parser, expression);
    if (result >= 0)
        lexer_next(&parser->lexer);
    return result;
}
static int binary(Parser *parser, ExprKind kind, int left, int right) {
    if (left < 0 || right < 0)
        return -1;
    Span span = {parser->nodes[left].span.start, parser->nodes[right].span.end};
    Expr expression = {kind, span, 0, left, right};
    return node(parser, expression);
}
static int product(Parser *parser) {
    int left = primary(parser);

    while (left >= 0 &&
           (parser->lexer.token.kind == TK_STAR)) {
        TokenKind op = parser->lexer.token.kind;
        lexer_next(&parser->lexer);
        int right = primary(parser);

        left = binary(parser, EX_MUL, left, right);
    }
    return left;
}
int parse_expression(Parser *parser) {
    int left = product(parser);

    while (left >= 0 &&
           (parser->lexer.token.kind == TK_PLUS || parser->lexer.token.kind == TK_MINUS)) {
        TokenKind op = parser->lexer.token.kind;
        lexer_next(&parser->lexer);
        int right = product(parser);

        left = binary(parser, op == TK_PLUS ? EX_ADD : EX_SUB, left, right);
    }
    return left;
}

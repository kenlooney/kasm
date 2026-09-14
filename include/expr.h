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

#ifndef KASM_EXPR_H
#define KASM_EXPR_H

#include "lexer.h"

typedef enum {
    EX_INT,
    EX_ADD,
    EX_SUB,
    EX_MUL
} ExprKind;

typedef struct {
    ExprKind kind;
    Span span;
    long long value;
    int left, right;
} Expr;

typedef struct {
    Lexer lexer;
    Expr *nodes;
    int count, depth, failed;
} Parser;

void parser_start(Parser *parser, const Source *source);
void parser_error(Parser *parser, const char *message);
int parse_expression(Parser *parser);

#endif // KASM_EXPR_H
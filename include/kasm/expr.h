/*
 * Copyright (C) 2026 Kenneth Looney
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef KASM_EXPR_H
#define KASM_EXPR_H
#include "kasm/lexer.h"

typedef enum { EX_INT, EX_ADD, EX_SUB, EX_MUL, EX_DIV, EX_MOD } ExprKind;
typedef struct {
    ExprKind kind;
    Span span;
    long long value;
    int left, right;
} Expr;

typedef struct {
    Lexer lexer;
    Expr *nodes;
    int count, capacity, failed;
} Parser;

void parser_start(Parser *parser, const Source *source);
void parser_error(Parser *parser, const char *message);
int parse_expression(Parser *parser);

#endif // KASM_EXPR_H
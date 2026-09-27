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
#ifndef KASM_PROGRAM_H
#define KASM_PROGRAM_H

#include "kasm/expr.h"

typedef enum {
    ST_MOV
} StatementKind;

typedef struct {
    StatementKind kind;
    Span span;
    Token operand;
    int expression;
    long long value;
    size_t offset;
} Statement;

typedef struct {
    Statement *statements;
    int count;
    int capacity;
} Program;

int parse_program(Parser *parser, Program *program);
int check_program(Parser *parser, Program *program);

#endif // KASM_PROGRAM_H

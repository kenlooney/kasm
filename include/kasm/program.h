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
#include "kasm/target.h"
#include <stddef.h>
#include <stdint.h>


typedef enum {
    ST_MOV,
    ST_HLT,
    ST_CLI,
    ST_STI,
    ST_LABEL,
    ST_JMP8,
    ST_ORG,
    ST_DB,
    ST_DW,
    ST_DD,
    ST_PADTO,
    ST_NOP,
    ST_CLC,
    ST_STC,
    ST_CMC,
    ST_CLD,
    ST_STD,
    ST_LAHF,
    ST_SAHF,
    ST_PUSHF,
    ST_POPF,
} StatementKind;

typedef struct {
    StatementKind kind;
    Span span;
    Token operand;
    Token label;
    Token target;

    int expression;
    int fill_expression;
    
    long long value;
    long long fill_value;

    size_t offset;
} Statement;

typedef struct {
    Statement *statements;
    int count;
    int capacity;
    uint64_t origin;
    int has_origin;
} Program;

int parse_program(Parser *parser, Program *program);
int evaluate_program(Parser *parser, Program *program);
int check_program(Parser *parser, Program *program, const Target *target);

#endif // KASM_PROGRAM_H

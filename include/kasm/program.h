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
    ST_MOV_SEGMENT,
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
    ST_CBW,
    ST_CWD,
    ST_IRET,
    ST_INC,
    ST_DEC,
    ST_PUSH,
    ST_POP,
    ST_XCHG,
} StatementKind;

typedef enum {
    REG16_AX = 0,
    REG16_CX = 1,
    REG16_DX = 2,
    REG16_BX = 3,
    REG16_SP = 4,
    REG16_BP = 5,
    REG16_SI = 6,
    REG16_DI = 7,
    REG16_INVALID = 8
} Register16;

typedef enum {
    SEG_ES = 0,
    SEG_CS = 1,
    SEG_SS = 2,
    SEG_DS = 3,
    SEG_FS = 4,
    SEG_GS = 5,
    SEG_INVALID = 6
} SegmentRegister;

typedef struct {
    StatementKind kind;
    Span span;
    Token operand;
    Token second_operand;
    Token label;
    Token target;

    int expression;
    int fill_expression;
    
    long long value;
    long long fill_value;

    size_t offset;
    Register16 reg16;
    SegmentRegister segment_register;
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

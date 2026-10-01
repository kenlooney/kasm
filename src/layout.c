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
#include "kasm/layout.h"

#include <stdint.h>

static int statement_size(
    const Statement *statement,
    const Target *target,
    size_t *size
) {
    if (target->arch != ARCH_X86 || target->mode != MODE_16)
        return 0;

    switch (statement->kind) {
    case ST_LABEL:
    case ST_ORG:
        *size = 0;
        return 1;
    case ST_CLI:
    case ST_STI:
    case ST_HLT:
    case ST_CLC:
    case ST_STC:
    case ST_CMC:
    case ST_CLD:
    case ST_STD:
    case ST_LAHF:
    case ST_SAHF:
    case ST_PUSHF:
    case ST_NOP:
    case ST_POPF:
    case ST_CBW:
    case ST_CWD:
    case ST_IRET:
    case ST_INC:
    case ST_DEC:
    case ST_PUSH:
    case ST_POP:
    case ST_XCHG:
    case ST_DB:
        *size = 1;
        return 1;    
    case ST_JMP8:
    case ST_DW:
    case ST_MOV_SEGMENT:
        *size = 2;
        return 1;
    case ST_MOV:
        *size = 3;
        return 1;
    case ST_DD:
        *size = 4;
        return 1;
    case ST_JMPFAR:
        *size = 5;
        return 1;   
    case ST_PADTO:
        if (statement->value < 0)
            return 0;

        if ((uint64_t)statement->value > (uint64_t)SIZE_MAX)
            return 0;

        if ((size_t)statement->value < statement->offset)
            return 0;

        *size = (size_t)statement->value - statement->offset;
        return 1;

    default:
        return 0;
    }
}

int layout_program(Program *program, const Target *target)
{
    if (!program || !target)
        return 0;

    size_t offset = 0;

    for (int i = 0; i < program->count; i++) {
        Statement *statement = &program->statements[i];
        size_t size;

        /*
         * Record where this statement begins before calculating its size.
         * Position-aware directives such as PADTO need the current offset.
         */
        statement->offset = offset;

        if (!statement_size(statement, target, &size))
            return 0;

        if (size > SIZE_MAX - offset)
            return 0;

        offset += size;
    }

    return 1;
}

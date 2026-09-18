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

#ifndef KASM_PROGRAM_H
#define KASM_PROGRAM_H

#define MAX_BLOCK_DEPTH 16 // Maximum allowed depth for nested blocks
#define KASM_IMAGE_LIMIT (4096u * 4096u)

#include "expr.h"
#include "target.h"
#include <stdint.h>
#include <stddef.h>

typedef enum { 
    ST_MOV, 
    ST_RET, 
    ST_LABEL, 
    ST_NEAR_JMP, 
    ST_SHORT_JMP, 
    ST_ABS_JMP,
    ST_DEC,
    ST_INC,
    ST_JNZ,
    ST_JZ,
    ST_JB,
    ST_JL,
    ST_ADD_RIM,
    ST_SUB_RIM,
    ST_PUSH,
    ST_PUSH_DS,
    ST_POP_DS,
    ST_PUSH_CS,
    ST_POP_CS,
    ST_PUSH_ES,
    ST_POP_ES,
    ST_PUSH_GS,
    ST_POP_GS,
    ST_PUSH_FS,
    ST_POP_FS,
    ST_PUSH_SS,
    ST_POP_SS,
    ST_POP,
    ST_OR,
    ST_AND,
    ST_XOR,
    ST_ADC, // Add with carry
    ST_SBB, // Subtract with borrow
    ST_INT_IMM8,
    ST_CMP_RIM,
    ST_DB,
    ST_DW,
    ST_DD,
    ST_DQ,
    ST_HALT,
    ST_PAUSE,

} StatementKind;

static inline int is_data_kind(StatementKind kind)
{
    return kind == ST_DB || kind == ST_DW || kind == ST_DD || kind == ST_DQ;
}

typedef struct {
    int expression; /* index into parser->nodes*/
    uint64_t value; /* filled by semantic checker */
} DataElement;

typedef struct {
    StatementKind kind;
    Span span;
    Token operand;
    int expression;
    long long value;
    size_t offset;
    unsigned reg_code;
    unsigned operand_bits; /* 16 for ax/cx/dx; 32 for eax/ecx/edx; 64 for rax/rcx/rdx */
    size_t data_start; /* index into program->data */
    size_t data_count; /* elements belonging to this statement */
    unsigned data_width; /* bytes per data element */
    int repeat_expression; /* expression-node index, or -1 for plain data */
    size_t repeat_count; /* number of times to repeat the data element */
    int is_memory_operand; /* 1 if the operand is a memory reference, 0 otherwise */
    Token base_operand;
} Statement;


typedef struct {
    Statement *statements;
    int count, capacity;
    DataElement *data;
    size_t data_count; /* elements across all data statements */
    size_t data_capacity; /* allocated capacity for data array */
} Program;
// Initializes a fresh Program. Caller frees statements even if parsing fails.
int parse_program(Parser *parser, Program *program);
int check_program(Parser *parser, Program *program, const Target *target);

#endif // KASM_PROGRAM_H

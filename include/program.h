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


#include "expr.h"
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
    ST_ADD_RIM,
    ST_SUB_RIM,
    ST_PUSH,
    ST_POP,
    ST_OR,
    ST_ADC,
    ST_INT_IMM8,

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
    int count, capacity;
} Program;
// Initializes a fresh Program. Caller frees statements even if parsing fails.
int parse_program(Parser *parser, Program *program);
int check_program(Parser *parser, Program *program);

#endif // KASM_PROGRAM_H

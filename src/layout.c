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

#include "layout.h"
#include <string.h>
static int same_name(const Source *source, Token a, Token b) {
    size_t n = a.span.end - a.span.start;
    return n == b.span.end - b.span.start &&
           memcmp(source->text + a.span.start, source->text + b.span.start, n) == 0;
}
size_t instruction_size(const Statement *statement) {
    switch (statement->kind) {
    case ST_MOV:
    case ST_ADD_RIM:
        return 5;
    case ST_SUB_RIM:
        return 5;
    case ST_RET:
        return 1;
    case ST_LABEL:
        return 0;
    case ST_NEAR_JMP:
        return 5;
    case ST_SHORT_JMP:
        return 2;
    case ST_ABS_JMP:
        return 14;
    case ST_DEC:
        return 2;
    case ST_INC:
        return 2;
    case ST_JNZ:
        return 6;
    case ST_JZ:
        return 6;
    }
    return 0;
}
int layout(const Source *source, Program *program) {
    size_t offset = 0;
    for (int i = 0; i < program->count; i++) {
        Statement *s = &program->statements[i];
        s->offset = offset;
        offset += instruction_size(s);
        if (s->kind != ST_LABEL)
            continue;
        for (int j = 0; j < i; j++) {
            Statement *previous = &program->statements[j];
            if (previous->kind == ST_LABEL && same_name(source, previous->operand, s->operand)) {
                diagnostic(source, s->operand.span, "duplicate label");
                return 0;
            }
        }
    }
    return 1;
}
int label_offset(const Source *source, const Program *program, Token name, size_t *offset) {
    for (int i = 0; i < program->count; i++) {
        const Statement *s = &program->statements[i];
        if (s->kind == ST_LABEL && same_name(source, s->operand, name)) {
            *offset = s->offset;
            return 1;
        }
    }
    diagnostic(source, name.span, "undefined label");
    return 0;
}



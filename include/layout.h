#ifndef KASM_LAYOUT_H
#define KASM_LAYOUT_H

#include "program.h"
#include "target.h"

size_t instruction_size(const Statement *statement);
int layout(const Source *source, const Parser *parser, Program *program, const Target *target);
int label_offset(const Source *source, const Program *program, Token name, size_t *offset);

#endif // KASM_LAYOUT_H
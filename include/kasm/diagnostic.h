#ifndef KASM_DIAGNOSTIC_H
#define KASM_DIAGNOSTIC_H

#include "kasm/source.h"
typedef struct {
    size_t start, end;
} Span;
void diagnostic(const Source *source, Span span, const char *message);

#endif // KASM_DIAGNOSTIC_H
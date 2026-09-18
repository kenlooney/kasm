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

#ifndef KASM_COFF_H
#define KASM_COFF_H

#include "emit.h"
#include <stdint.h>

typedef struct {
    const char *name;
    uint32_t value;       /* offset within .text; zero for undefined symbols */
    int16_t section;      /* 1 = .text, 0 = undefined */
    unsigned exported;   /* visible to the linker when nonzero */
} CoffSymbol;

typedef enum {
    COFF_ADDR64 = 0x0001,
    COFF_REL32 = 0x0004
} CoffRelocationKind;

typedef struct {
    uint32_t offset;      /* section-relative start of the address field */
    uint32_t symbol;      /* zero-based symbol-table index */
    CoffRelocationKind kind;
} CoffRelocation;

typedef struct {
    const Bytes *text;
    const CoffSymbol *symbols;
    size_t symbol_count;
    const CoffRelocation *relocations;
    size_t relocation_count;
} CoffObject;

/* Borrows all input storage during the call; returns 1 on success, 0 on failure.
   The first backend supports one .text section, REL32, and ADDR64. */
int write_coff(const CoffObject *object, const char *path);
#endif // KASM_COFF_H

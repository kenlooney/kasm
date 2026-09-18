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

#include "coff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int number(Bytes *out, uint64_t value, unsigned width)
{
    for (unsigned i = 0; i < width; ++i)
        if (!byte_push(out, (unsigned char)(value >> (8 * i)))) return 0;
    return 1;
}

static int append(Bytes *out, const void *data, size_t count)
{
    const unsigned char *p = data;
    for (size_t i = 0; i < count; ++i)
        if (!byte_push(out, p[i])) return 0;
    return 1;
}

static int validate(const CoffObject *o)
{
    if (!o || !o->text || o->text->count > KASM_IMAGE_LIMIT ||
        o->text->patch_count != 0 ||
        (o->text->count && !o->text->data) ||
        (o->symbol_count && !o->symbols) ||
        (o->relocation_count && !o->relocations) ||
        o->symbol_count > KASM_IMAGE_LIMIT / 18 ||
        o->relocation_count > UINT16_MAX) return 0;
    for (size_t i = 0; i < o->symbol_count; ++i) {
        const CoffSymbol *s = &o->symbols[i];
        if (!s->name || !*s->name || strlen(s->name) >= KASM_IMAGE_LIMIT ||
            (s->section != 0 && s->section != 1) ||
            (s->section == 0 && (s->value != 0 || !s->exported)) ||
            (s->section == 1 && s->value > o->text->count)) return 0;
        for (size_t j = 0; j < i; ++j)
            if (strcmp(s->name, o->symbols[j].name) == 0) return 0;
    }
    for (size_t i = 0; i < o->relocation_count; ++i) {
        const CoffRelocation *r = &o->relocations[i];
        size_t width = r->kind == COFF_ADDR64 ? 8 : r->kind == COFF_REL32 ? 4 : 0;
        if (!width || r->symbol >= o->symbol_count ||
            r->offset > o->text->count || width > o->text->count - r->offset)
            return 0;
        for (size_t j = 0; j < i; ++j) {
            const CoffRelocation *previous = &o->relocations[j];
            size_t previous_width = previous->kind == COFF_ADDR64 ? 8 : 4;
            if (r->offset < previous->offset + previous_width &&
                previous->offset < r->offset + width) return 0;
        }
    }
    return 1;
}

int write_coff(const CoffObject *object, const char *path)
{
    if (!path || !validate(object)) {
        fputs("invalid or unsupported COFF object\n", stderr); return 0;
    }
    const Bytes *text = object->text;
    Bytes file = {0}, symbols = {0}, strings = {0};
    int ok = 0;
    if (!number(&strings, 0, 4)) goto done;
    for (size_t i = 0; i < object->symbol_count; ++i) {
        const CoffSymbol *s = &object->symbols[i];
        size_t length = strlen(s->name);
        if (length <= 8) {
            if (!append(&symbols, s->name, length) ||
                !number(&symbols, 0, (unsigned)(8 - length))) goto done;
        } else {
            if (!number(&symbols, 0, 4) || !number(&symbols, strings.count, 4) ||
                !append(&strings, s->name, length + 1)) goto done;
        }
        if (!number(&symbols, s->value, 4) ||
            !number(&symbols, (uint16_t)s->section, 2) ||
            !number(&symbols, 0x20, 2) || /* function symbol */
            !number(&symbols, s->exported ? 2 : 3, 1) ||
            !number(&symbols, 0, 1)) goto done; /* no auxiliary records */
    }
    for (unsigned i = 0; i < 4; ++i)
        strings.data[i] = (unsigned char)(strings.count >> (8 * i));

    size_t reloc_offset = 60 + text->count;
    size_t symbol_offset = reloc_offset + object->relocation_count * 10;
    if (symbol_offset > KASM_IMAGE_LIMIT ||
        symbols.count > KASM_IMAGE_LIMIT - symbol_offset ||
        strings.count > KASM_IMAGE_LIMIT - symbol_offset - symbols.count) {
        fputs("COFF file exceeds configured image limit\n", stderr); goto done;
    }
    /* 20-byte file header. */
    if (!number(&file, 0x8664, 2) || !number(&file, 1, 2) ||
        !number(&file, 0, 4) || !number(&file, symbol_offset, 4) ||
        !number(&file, object->symbol_count, 4) ||
        !number(&file, 0, 2) || !number(&file, 0, 2)) goto done;
    /* 40-byte section header. */
    if (!append(&file, ".text\0\0\0", 8) ||
        !number(&file, 0, 4) || !number(&file, 0, 4) ||
        !number(&file, text->count, 4) || !number(&file, 60, 4) ||
        !number(&file, object->relocation_count ? reloc_offset : 0, 4) ||
        !number(&file, 0, 4) || !number(&file, object->relocation_count, 2) ||
        !number(&file, 0, 2) || !number(&file, 0x60500020, 4) ||
        !append(&file, text->data, text->count)) goto done;
    for (size_t i = 0; i < object->relocation_count; ++i) {
        const CoffRelocation *r = &object->relocations[i];
        if (!number(&file, r->offset, 4) || !number(&file, r->symbol, 4) ||
            !number(&file, r->kind, 2)) goto done;
    }
    if (!append(&file, symbols.data, symbols.count) ||
        !append(&file, strings.data, strings.count)) goto done;
    ok = write_binary(&file, path);
done:
    free(strings.data); free(symbols.data); free(file.data);
    return ok;
}

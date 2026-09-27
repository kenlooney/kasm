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

#ifndef KASM_CURSOR_H
#define KASM_CURSOR_H

#include "kasm/source.h"
typedef struct {
    const Source *source;
    size_t offset, line, column;
} Cursor;

Cursor cursor_start(const Source *source);
char cursor_peek(const Cursor *cursor);
void cursor_advance(Cursor *cursor);

#endif // KASM_CURSOR_H

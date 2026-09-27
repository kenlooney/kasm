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

#include "kasm/cursor.h"
Cursor cursor_start(const Source *source) {
    Cursor cursor = {source, 0, 1, 1};
    return cursor;
}
char cursor_peek(const Cursor *cursor) {
    return cursor->source->text[cursor->offset];
}
void cursor_advance(Cursor *cursor) {
    char c = cursor_peek(cursor);
    if (c == '\0')
        return;
    cursor->offset++;
    if (c == '\n') {
        cursor->line++;
        cursor->column = 1;
    } else {
        cursor->column++;
    }
}

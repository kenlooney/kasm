/*
 * This file is part of KASM.
 *
 * KASM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KASM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KASM.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef KASM_EMIT_H
#define KASM_EMIT_H

#include <stddef.h>
#include <stdint.h>

#define KASM_BYTES_INITIAL_CAPACITY 256u

typedef struct {
    uint8_t *data;
    size_t count;
    size_t capacity;
} Bytes;

int byte_push(Bytes *bytes, uint8_t value);
int little_endian(Bytes *bytes, uint64_t value, size_t width);
void bytes_free(Bytes *bytes);

#endif // KASM_EMIT_H

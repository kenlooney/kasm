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
#include "emit.h"
#include <stdio.h>
#include <stdlib.h>

int byte_push(Bytes *bytes, unsigned char value)
{
    if (bytes->count >= KASM_IMAGE_LIMIT) {
        fprintf(stderr, "image exceeds image limit\n");
        return 0;
    }
    if (bytes->count == bytes->capacity) {
        size_t capacity = bytes->capacity == 0 ? 16 :
            bytes->capacity > KASM_IMAGE_LIMIT / 2 ? KASM_IMAGE_LIMIT :
            bytes->capacity * 2;
        unsigned char *data = realloc(bytes->data, capacity);
        if (data == NULL) {
            fprintf(stderr, "failed to allocate memory for machine code\n");
            return 0;
        }
        bytes->data = data;
        bytes->capacity = capacity;
    }
    bytes->data[bytes->count++] = value;
    return 1;
}


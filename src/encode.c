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


#include "source.h"
#include "program.h"
#include "emit.h"
#include <stdint.h>

static int little_endian(Bytes *bytes, uint64_t value, int width) {
    for (int i = 0; i < width; i++) {
        if (!byte_push(bytes, (unsigned char)(value & 255)))
            return 0;
        value >>= 8;
    }
    return 1;
}
int encode(const Source *source, Program *program, Bytes *bytes) {
    (void)source;
    for (int i = 0; i < program->count; i++) {
        Statement *s = &program->statements[i];
        if (s->kind == ST_MOV) {
            if (!byte_push(bytes, 0xB8) || !little_endian(bytes, (uint64_t)s->value, 4))
                return 0;
        }
    }
    return 1;
}

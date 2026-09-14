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


#ifndef KASM_EMIT_H
#define KASM_EMIT_H

#include "program.h"

typedef struct {
    unsigned char *data;
    int count;
    int capacity;
} Bytes;

int byte_push(Bytes *bytes, unsigned char value);
int encode(const Source *source, Program *program, Bytes *bytes);

#endif // KASM_EMIT_H
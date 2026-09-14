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

#ifndef KASM_SOURCE_H
#define KASM_SOURCE_H

#include <stddef.h>

typedef struct {
    const char *path;
    char text[4097];
    size_t length;
} Source;

int source_load(Source *source, const char *path);



#endif // KASM_SOURCE_H
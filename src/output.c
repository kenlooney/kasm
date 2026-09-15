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

#include "output.h"
#include <stdio.h>



int write_c(const Bytes *bytes, const char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror(path);
        return 0;
    }
    fprintf(file, "#ifndef GENERATED_H\n#define GENERATED_H\n#include <stddef.h>\n");
    fprintf(file, "static const unsigned char code[] = {\n");
    if (bytes->count == 0)
        fprintf(file, "0\n");
    for (size_t i = 0; i < bytes->count; i++)
        fprintf(file, "0x%02X,\n", (unsigned int)bytes->data[i]);
    fprintf(file, "};\nstatic const size_t code_size = %zu;\n", bytes->count);

    fprintf(file, "static const size_t patch_offsets[] = {\n");
    if (bytes->patch_count == 0)
        fprintf(file, "0\n");
    for (size_t i = 0; i < bytes->patch_count; i++)
        fprintf(file, "%zu,\n", bytes->patches[i]);
    fprintf(file, "};\nstatic const size_t patch_count = %zu;\n", bytes->patch_count);

    fprintf(file, "#endif\n");
    int okay = !ferror(file);
    if (fclose(file) != 0)
        okay = 0;
    if (!okay)
        fprintf(stderr, "%s: write failed\n", path);
    return okay;
}


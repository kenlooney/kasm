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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int source_load(Source *source, const char *path) {
    FILE *file = fopen(path, "rb");
    source->path = path;
    source->text = NULL;
    source->length = 0;
    if (file == NULL) {
        perror(path);
        return 0;
    }
    size_t capacity = 1;
    source->text = malloc(capacity);
    if (source->text == NULL) {
        fprintf(stderr, "%s: failed to allocate source buffer\n", path);
        fclose(file);
        return 0;
    }
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == 0 || c > 127) {
            fprintf(stderr, "%s: expected NUL-free ASCII source\n", path);
            fclose(file);
            source_free(source);
            return 0;
        }
        if (source->length == capacity - 1) {
            if (capacity > SIZE_MAX / 2) {
                fprintf(stderr, "%s: source file is too large\n", path);
                fclose(file);
                source_free(source);
                return 0;
            }
            capacity *= 2;
            char *text = realloc(source->text, capacity);
            if (text == NULL) {
                fprintf(stderr, "%s: failed to grow source buffer\n", path);
                fclose(file);
                source_free(source);
                return 0;
            }
            source->text = text;
        }
        source->text[source->length++] = (char)c;
    }
    int failed = ferror(file);
    if (fclose(file) != 0)
        failed = 1;
    source->text[source->length] = '\0';
    if (failed) {
        fprintf(stderr, "%s: read failed\n", path);
        source_free(source);
    }
    return !failed;
}

void source_free(Source *source) {
    free(source->text);
    source->text = NULL;
    source->length = 0;
}

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
int source_load(Source *source, const char *path) {
    FILE *file = fopen(path, "rb");
    source->path = path;
    source->length = 0;
    if (file == NULL) {
        perror(path);
        return 0;
    }
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (source->length == 4096 || c == 0 || c > 127) {
            fprintf(stderr, "%s: expected at most 4096 non-NUL ASCII bytes\n", path);
            fclose(file);
            return 0;
        }
        source->text[source->length++] = (char)c;
    }
    int failed = ferror(file);
    if (fclose(file) != 0)
        failed = 1;
    source->text[source->length] = '\0';
    if (failed)
        fprintf(stderr, "%s: read failed\n", path);
    return !failed;
}

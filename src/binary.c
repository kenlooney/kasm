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

int write_binary(const Bytes *bytes, const char *path) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        perror(path);
        return 0;
    }
    int okay = fwrite(bytes->data, 1, bytes->count, file) == bytes->count;
    if (fclose(file) != 0)
        okay = 0;
    if (!okay)
        fprintf(stderr, "%s: write failed\n", path);
    return okay;
}

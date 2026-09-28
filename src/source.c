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
#include "kasm/source.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int source_load(Source *source, const char *path) {
    char *text;
    long file_length;

    if (!source || !path) {
        return -1;
    }

    FILE *file = fopen(path, "rb");
    if (!file) {
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0 ||
        (file_length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return -1;
    }

    text = (char *)malloc((size_t)file_length + 1);
    if (!text) {
        fclose(file);
        return -1;
    }

    if (fread(text, 1, (size_t)file_length, file) != (size_t)file_length) {
        fclose(file);
        free(text);
        return -1;
    }

    if (fclose(file) != 0) {
        free(text);
        return -1;
    }

    text[file_length] = '\0';
    source->path = path;
    source->text = text;
    source->length = (size_t)file_length;

    return 0;
}

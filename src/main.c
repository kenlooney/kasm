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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "source.h"
#include "cursor.h"
#include "diagnostic.h"
#include "lexer.h"
#include "expr.h"
#include "program.h"
#include "emit.h"
#include "layout.h"
#include "relocate.h"
#include "output.h"
#include "decode.h"
#include "target.h"
#include "coff.h"
/* Use the final filename component and replace only its last extension.
   Raw outputs remain in the current working directory. */
static char *raw_output_name(const char *source_path, const char *extension)
{
    const char *name = source_path;
    for (const char *p = source_path; *p; ++p)
        if (*p == '/' || *p == '\\') name = p + 1;
    const char *dot = strrchr(name, '.');
    size_t length = dot && dot != name ? (size_t)(dot - name) : strlen(name);
    char *path = malloc(length + strlen(extension) + 1);
    if (path != NULL)
    {
        memcpy(path, name, length);
        strcpy(path + length, extension);
    }
    return path;
}
int main(int argc, char **argv)
{
    unsigned char demonstration[16] = {8};
    size_t patch = 0;
    if (!relocate(demonstration, sizeof demonstration, &patch, 1))
        return 1;

    Source source;
    Target target;
    const char *source_path;
    const char *coff_export = NULL;
    const char *output_path = NULL;
    if (argc == 2)
    {
        target.mode = MODE_64;
        source_path = argv[1];
    }
    else if (argc == 4 && strcmp(argv[1], "--bits") == 0)
    {
        if (strcmp(argv[2], "16") == 0)
            target.mode = MODE_16;
        else if (strcmp(argv[2], "32") == 0)
            target.mode = MODE_32;
        else if (strcmp(argv[2], "64") == 0)
            target.mode = MODE_64;
        else
        {
            fprintf(stderr, "Invalid target mode: %s\n", argv[2]);
            return 1;
        }
        source_path = argv[3];
    }
    else if (argc == 8 && strcmp(argv[1], "--format") == 0 &&
             strcmp(argv[2], "coff") == 0 && strcmp(argv[3], "--export") == 0 &&
             strcmp(argv[5], "-o") == 0 && argv[6][0] != '\0')
    {
        target.mode = MODE_64;
        coff_export = argv[4];
        output_path = argv[6];
        source_path = argv[7];
    }
    else
    {
        fprintf(stderr, "Usage: %s [--bits <target-mode>] <source-file>\n", argv[0]);
        fprintf(stderr, "       %s --format coff --export <label> -o <output-path> <source-file>\n", argv[0]);
        return 1;
    }

    if (!source_load(&source, source_path))
        return 1;

    Parser parser;
    Program program;
    parser_start(&parser, &source);
    if (!parse_program(&parser, &program))
    {
        free(program.data);
        free(program.statements);
        free(parser.nodes);
        source_free(&source);
        return 1;
    }
    if (!check_program(&parser, &program, &target))
    {
        free(program.data);
        free(parser.nodes);
        free(program.statements);
        source_free(&source);
        return 1;
    }
    if (!layout(&source, &parser, &program, &target))
    {
        free(parser.nodes);
        free(program.data);
        free(program.statements);
        source_free(&source);
        return 1;
    }
    if (!resolve_data_values(&source, &parser, &program))
    {
        free(parser.nodes);
        free(program.data);
        free(program.statements);
        source_free(&source);
        return 1;
    }
    
    Bytes bytes = {0};
    if (!encode(&source, &program, &bytes, &target))
    {
        free(bytes.data);
        free(parser.nodes);
        free(program.data);
        free(program.statements);
        source_free(&source);
        return 1;
    }
    if (coff_export != NULL)
    {
        int okay = 1, found = 0;
        size_t entry_offset = 0;
        for (int i = 0; i < program.count; ++i)
        {
            const Statement *s = &program.statements[i];
            if (is_data_kind(s->kind))
                okay = 0;
            if (s->kind == ST_LABEL && token_is(&source, s->operand, coff_export))
            {
                found = 1;
                entry_offset = s->offset;
            }
        }
        if (!okay || !found || entry_offset >= bytes.count)
        {
            fprintf(stderr, "COFF requires an instruction-only image and a defined export\n");
            okay = 0;
        }
        else
        {
            CoffSymbol entry = {coff_export, (uint32_t)entry_offset, 1, 1};
            CoffObject object = {&bytes, &entry, 1, NULL, 0};
            okay = write_coff(&object, output_path);
        }
        free(bytes.data);
        free(parser.nodes);
        free(program.data);
        free(program.statements);
        source_free(&source);
        return okay ? 0 : 1;
    }
    for (size_t i = 0; i < bytes.count; i++)
        printf("%02X ", (unsigned int)bytes.data[i]);
    puts("");
    char *binary_path = raw_output_name(source_path, ".bin");
    char *header_path = raw_output_name(source_path, ".h");
    int written = 0;
    if (binary_path == NULL || header_path == NULL)
        fprintf(stderr, "Failed to allocate output filenames\n");
    else
        written = write_binary(&bytes, binary_path) && write_c(&bytes, header_path);
    free(binary_path);
    free(header_path);
    if (!written)
    {
        free(bytes.data);
        free(parser.nodes);
        free(program.data);
        free(program.statements);
        source_free(&source);
        return 1;
    }
    if (program.data_count != 0)
    {
        puts("Data emitted; instruction-only decoding skipped.");
    }
    else
    {
        if (!decode(&bytes, &target))
        {
            free(bytes.data);
            free(parser.nodes);
            free(program.data);
            free(program.statements);
            source_free(&source);
            return 1;
        }
        puts("Decoding successful.");
    }
    free(bytes.data);
    free(parser.nodes);
    free(program.data);
    free(program.statements);

    source_free(&source);

    return 0;
}

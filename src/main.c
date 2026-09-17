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
int main(int argc, char **argv)
{
    unsigned char demonstration[16] = {8};
    size_t patch = 0;
    if (!relocate(demonstration, sizeof demonstration, &patch, 1))
        return 1;

    Source source;
    Target target;
    const char *source_path;
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
    else
    {
        fprintf(stderr, "Usage: %s [--bits <target-mode>] <source-file>\n", argv[0]);
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
    for (size_t i = 0; i < bytes.count; i++)
        printf("%02X ", (unsigned int)bytes.data[i]);
    puts("");
    if (!write_binary(&bytes, "program.bin"))
    {
        free(bytes.data);
        free(parser.nodes);
        free(program.data);
        free(program.statements);
        source_free(&source);
        return 1;
    }
    if (!write_c(&bytes, "generated.h"))
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
        if (!decode(&bytes))
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

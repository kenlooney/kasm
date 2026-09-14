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

int main(int argc, char **argv)
{
    Source source;
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }
    if (!source_load(&source, argv[1]))
        return 1;

    Parser parser;
    Program program;
    parser_start(&parser, &source);
    if (!parse_program(&parser, &program))
        return 1;
    if (!check_program(&parser, &program))
        return 1;

    Bytes bytes = {0};
    if (!encode(&source, &program, &bytes))
        return 1;
    for (size_t i = 0; i < bytes.count; i++)
        printf("%02X ", (unsigned int)bytes.data[i]);
    free(program.statements);
    puts("");
    return 0;
}

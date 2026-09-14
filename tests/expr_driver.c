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

int main(int argc, char** argv){
    Source source;
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }
     if (!source_load(&source, argv[1]))
        return 1;

   Parser parser;
    parser_start(&parser, &source);
    int root = parse_expression(&parser);
    while (!parser.lexer.failed && parser.lexer.token.kind == TK_NEWLINE)
        lexer_next(&parser.lexer);
    if (root >= 0 && parser.lexer.token.kind != TK_END)
        parser_error(&parser, "expected end of input");
    if (root < 0 || parser.failed || parser.lexer.failed) {
        free(parser.nodes);
        return 1;
    }
    for (int i = 0; i < parser.count; i++) {
        Expr e = parser.nodes[i];
        printf("node %d: kind=%d value=%lld left=%d right=%d\n", i, (int)e.kind, e.value, e.left,
               e.right);
    }
    printf("root = %d\n", root);
    free(parser.nodes);
    return 0;
}

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

#ifndef KASM_LEXER_H
#define KASM_LEXER_H

#include "source.h"
#include "cursor.h"
#include "diagnostic.h"
typedef enum {
    TK_END,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_LPAREN,
    TK_RPAREN,
    TK_COMMA,
    TK_SEMI,
    TK_LBRACE,
    TK_RBRACE,
    TK_COLON,
    TK_IDENT,
    TK_NUMBER,
    TK_NEWLINE
} TokenKind;

typedef struct {
    TokenKind kind;
    Span span;
    long long value;
} Token;

typedef struct {
    Cursor cursor;
    Token token;
    int failed;
} Lexer;

void lexer_start(Lexer *lexer, const Source *source);
void lexer_next(Lexer *lexer);
int token_is(const Source *source, Token token, const char *word);
#endif // KASM_LEXER_H

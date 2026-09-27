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
#ifndef KASM_LEXER_H
#define KASM_LEXER_H

#include "kasm/source.h"
#include "kasm/diagnostic.h"
#include "kasm/cursor.h"
typedef enum {
    TK_END,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_SLASH,
    TK_PERCENT,
    TK_LPAREN,
    TK_RPAREN,
    TK_COMMA,
    TK_SEMI,
    TK_LBRACE,
    TK_RBRACE,
    TK_COLON,
    TK_IDENT,
    TK_NUMBER,
    
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
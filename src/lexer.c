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
#include "kasm/lexer.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * Check if the given token matches the specified word.
 *
 * @param source The source code containing the token.
 * @param token The token to check.
 * @param word The word to compare against the token.
 * @return Non-zero if the token matches the word, zero otherwise.
 */
int token_is(const Source *source, Token token, const char *word)
{
    size_t length = token.span.end - token.span.start;
    return length == strlen(word) && memcmp(source->text + token.span.start, word, length) == 0;
}

/**
 * Initialize the lexer with the given source code.
 *
 * @param lexer The lexer to initialize.
 * @param source The source code to lex.
 */
void lexer_start(Lexer *lexer, const Source *source)
{
    lexer->cursor = cursor_start(source);
    lexer->failed = 0;
    lexer_next(lexer);
}

void lexer_next(Lexer *lexer)
{
    Cursor *cursor = &lexer->cursor;
    Token token = {TK_END, {cursor->offset, cursor->offset}, 0};

    // If the lexer has previously failed, return the end token immediately.
    if (lexer->failed)
    {
        lexer->token = token;
        return;
    }

    char c = cursor_peek(cursor); // Peek at the current character in the source code.

    if (c == '\0')
    {
        token.span.end = cursor->offset;
        lexer->token = token;
        return;
    }

    // Handle identifiers
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
    {
        token.kind = TK_IDENT;
        while ((cursor_peek(cursor) >= 'a' && cursor_peek(cursor) <= 'z') ||
               (cursor_peek(cursor) >= 'A' && cursor_peek(cursor) <= 'Z') ||
               (cursor_peek(cursor) >= '0' && cursor_peek(cursor) <= '9') ||
               cursor_peek(cursor) == '_')
        {
            cursor_advance(cursor);
        }
        token.span.end = cursor->offset;
        lexer->token = token;
        return;
    }
    cursor_advance(cursor); // Advance the cursor to the next character in the source code.
    switch (c)
    {
    // Single character tokens (e.g., punctuation)
    case '+':
        token.kind = TK_PLUS;
        break;
    case '-':
        token.kind = TK_MINUS;
        break;
    case '*':
        token.kind = TK_STAR;
        break;
    case '/':
        token.kind = TK_SLASH;
        break;
    case '%':
        token.kind = TK_PERCENT;
        break;
    case '(':
        token.kind = TK_LPAREN;
        break;
    case ')':
        token.kind = TK_RPAREN;
        break;
    case ',':
        token.kind = TK_COMMA;
        break;
    case ';':
        token.kind = TK_SEMI;
        break;
    case '{':
        token.kind = TK_LBRACE;
        break;
    case '}':
        token.kind = TK_RBRACE;
        break;
    case ':':
        token.kind = TK_COLON;
        break;
    default:
    {
        lexer->failed = 1;
    }
    break;
    }
    token.span.end = cursor->offset;
    lexer->token = token;
    if (lexer->failed)
      diagnostic(cursor->source, token.span, "invalid character or integer out of range");
}

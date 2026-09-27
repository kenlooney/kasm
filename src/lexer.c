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
#include <limits.h>
#include <string.h>

static int digit_value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static char cursor_peek_next(const Cursor *cursor)
{
    return cursor->source->text[cursor->offset + 1];
}

static int skip_trivia(Lexer *lexer)
{
    Cursor *cursor = &lexer->cursor;

    for (;;)
    {
        char c = cursor_peek(cursor);

        while (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            cursor_advance(cursor);
            c = cursor_peek(cursor);
        }

        if (c == '/' && cursor_peek_next(cursor) == '/')
        {
            cursor_advance(cursor);
            cursor_advance(cursor);
            while (cursor_peek(cursor) != '\0' && cursor_peek(cursor) != '\n')
                cursor_advance(cursor);
            continue;
        }

        if (c == '/' && cursor_peek_next(cursor) == '*')
        {
            size_t comment_start = cursor->offset;
            cursor_advance(cursor);
            cursor_advance(cursor);

            while (cursor_peek(cursor) != '\0' &&
                   !(cursor_peek(cursor) == '*' && cursor_peek_next(cursor) == '/'))
            {
                cursor_advance(cursor);
            }

            if (cursor_peek(cursor) == '\0')
            {
                Span span = {comment_start, cursor->offset};
                lexer->failed = 1;
                lexer->token = (Token){TK_END, span, 0};
                diagnostic(cursor->source, span, "unterminated block comment");
                return 0;
            }

            cursor_advance(cursor);
            cursor_advance(cursor);
            continue;
        }

        return 1;
    }
}

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

    // If the lexer has previously failed, return the end token immediately.
    if (lexer->failed)
    {
        lexer->token = (Token){TK_END, {cursor->offset, cursor->offset}, 0};
        return;
    }

    if (!skip_trivia(lexer))
        return;

    Token token = {TK_END, {cursor->offset, cursor->offset}, 0};
    char c = cursor_peek(cursor);

    if (c == '\0')
    {
        lexer->token = token;
        return;
    }

    // Handle decimal, hexadecimal, and binary integer literals.
    if (c >= '0' && c <= '9')
    {
        int base = 10;
        char next = cursor_peek_next(cursor);

        if (c == '0' && (next == 'x' || next == 'X' || next == 'b' || next == 'B'))
        {
            cursor_advance(cursor);
            base = (next == 'x' || next == 'X') ? 16 : 2;
            cursor_advance(cursor);
        }

        int saw_digit = 0;
        while (!lexer->failed)
        {
            int digit = digit_value(cursor_peek(cursor));
            if (digit >= 0 && digit < base)
            {
                if (token.value > (LLONG_MAX - digit) / base)
                {
                    lexer->failed = 1;
                    break;
                }
                token.value = token.value * base + digit;
                saw_digit = 1;
                cursor_advance(cursor);
                continue;
            }

            if (cursor_peek(cursor) == '_')
            {
                int next_digit = digit_value(
                    cursor->source->text[cursor->offset + 1]);
                if (!saw_digit || next_digit < 0 || next_digit >= base)
                    lexer->failed = 1;
                cursor_advance(cursor);
                continue;
            }

            break;
        }

        token.kind = TK_NUMBER;
        token.span.end = cursor->offset;
        lexer->token = token;

        if (!saw_digit || lexer->failed)
        {
            lexer->failed = 1;
            diagnostic(cursor->source, token.span, "invalid character or integer out of range");
        }
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

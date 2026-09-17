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

#include "lexer.h"
#include <limits.h>
#include <string.h>

static int digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
int token_is(const Source *source, Token token, const char *word) {
    size_t length = token.span.end - token.span.start;
    return length == strlen(word) && memcmp(source->text + token.span.start, word, length) == 0;
}
void lexer_start(Lexer *lexer, const Source *source) {
    lexer->cursor = cursor_start(source);
    lexer->failed = 0;
    lexer_next(lexer);
}
void lexer_next(Lexer *lexer) {
    Cursor *cursor = &lexer->cursor;
    Token token = {TK_END, {cursor->offset, cursor->offset}, 0};
    if (lexer->failed) {
        lexer->token = token;
        return;
    }

    char c;
    // Skip trivia iteratively so adjacent comments do not grow the call stack.
    for (;;) {
        c = cursor_peek(cursor);
        while (c == ' ' || c == '\t' ||
               c == '\v' || c == '\f') {
            cursor_advance(cursor);
            c = cursor_peek(cursor);
        }
        token.span.start = token.span.end = cursor->offset;
        if (c == '\0') {
            token.span.end = cursor->offset;
            lexer->token = token;
            return;
        }
        // Check for single-line comments
        if (c == '/' && cursor->source->text[cursor->offset + 1] == '/') {
            while (c != '\0' && c != '\n' && c != '\r') {
                cursor_advance(cursor);
                c = cursor_peek(cursor);
            }
            continue;
        }
        // Check for multi-line comments
        if (c == '/' && cursor->source->text[cursor->offset + 1] == '*') {
            cursor_advance(cursor);
            cursor_advance(cursor);
            c = cursor_peek(cursor);
            while (c != '\0' && !(c == '*' && cursor->source->text[cursor->offset + 1] == '/')) {
                cursor_advance(cursor);
                c = cursor_peek(cursor);
            }
            if (c == '\0') {
                token.span.end = cursor->offset;
                lexer->failed = 1;
                lexer->token = token;
                diagnostic(cursor->source, token.span, "unterminated block comment");
                return;
            }
            cursor_advance(cursor);
            cursor_advance(cursor);
            continue;
        }
        break;
    }

    // Treat CRLF as one logical newline while preserving both source bytes.
    if (c == '\r' || c == '\n') {
        token.kind = TK_NEWLINE;
        cursor_advance(cursor);
        if (c == '\r' && cursor_peek(cursor) == '\n')
            cursor_advance(cursor);
        token.span.end = cursor->offset;
        lexer->token = token;
        return;
    }
    
    // Check for identifier
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        token.kind = TK_IDENT;
        while ((cursor_peek(cursor) >= 'a' && cursor_peek(cursor) <= 'z') ||
               (cursor_peek(cursor) >= 'A' && cursor_peek(cursor) <= 'Z') ||
               (cursor_peek(cursor) >= '0' && cursor_peek(cursor) <= '9') ||
               cursor_peek(cursor) == '_') {
            cursor_advance(cursor);
        }
        token.span.end = cursor->offset;
        lexer->token = token;
        return;
    }
    // Check for decimal, hexadecimal, binary, and octal integers
    if (c >= '0' && c <= '9') {
        int base = 10;
        size_t digits_start;
        token.kind = TK_NUMBER;
        if (c == '0') {
            base = 8;
            cursor_advance(cursor);
            c = cursor_peek(cursor);
            if (c == 'x' || c == 'X') {
                base = 16;
                cursor_advance(cursor);
            } else if (c == 'b' || c == 'B') {
                base = 2;
                cursor_advance(cursor);
            }
        }
        digits_start = cursor->offset;
        for (;;) {
            int digit;
            c = cursor_peek(cursor);
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') || c == '_'))
                break;
            digit = digit_value(c);
            if (digit < 0 || digit >= base) {
                lexer->failed = 1;
            } else if (!lexer->failed) {
                if (token.value > (LLONG_MAX - digit) / base)
                    lexer->failed = 1;
                else
                    token.value = token.value * base + digit;
            }
            cursor_advance(cursor);
        }
        if ((base == 16 || base == 2) && cursor->offset == digits_start)
            lexer->failed = 1;
        token.span.end = cursor->offset;
        lexer->token = token;
        if (lexer->failed)
            diagnostic(cursor->source, token.span, "invalid character or integer out of range");
        return;
    }
    // Check for single-character tokens
    cursor_advance(cursor);
    switch (c) {
    case '+':
        token.kind = TK_PLUS;
        break;
    case '-':
        token.kind = TK_MINUS;
        break;
    case '*':
        token.kind = TK_STAR;
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
    case '$':
        token.kind = TK_DOLLAR;
        break;
    default:

    {
        lexer->failed = 1;
    } break;
    }
    token.span.end = cursor->offset;
    lexer->token = token;
    if (lexer->failed)
        diagnostic(cursor->source, token.span, "invalid character or integer out of range");
}

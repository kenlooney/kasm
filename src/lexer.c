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
#include <string.h>
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

    char c = cursor_peek(cursor);
    if (c == '\0') {
        token.span.end = cursor->offset;
        lexer->token = token;
        return;
    }
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

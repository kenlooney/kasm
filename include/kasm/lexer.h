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
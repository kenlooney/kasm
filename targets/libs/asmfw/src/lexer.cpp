#include "framework/lexer.hpp"

bool Lexer::letter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool Lexer::digit(char ch) {
    return ch >= '0' && ch <= '9';
}
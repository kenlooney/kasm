#pragma once
#include "token.hpp"
#include <vector>

class Lexer {
public:
    virtual ~Lexer() = default;
    virtual std::vector<Token> tokenize(const std::string& text) = 0;
    bool letter(char ch);
    bool digit(char ch);
private:
    std::vector<Token> tokens_;
public:
    const std::vector<Token>& tokens() const { return tokens_; }
};